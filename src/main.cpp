#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include "time.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

#include "wifi_manager.h"
#include "servo_controller.h"
#include "battery_monitor.h"
#include "feeding_scheduler.h"
#include "api_handlers.h"
#include "oled_display.h"
#include "device_runtime.h"
#include "power_manager.h"
#include "ota_handler.h"
#include "version_info.h"

// === Hardware pins ===
const int SERVO_PIN = 4;
const int BUTTON_PIN = 3;
const int BATTERY_PIN = 2; // ⚡ MH Electronic Voltage Sensor (VOUT)
const int LIGHT_PIN = -1;  // GPIO5 is ADC2 — not supported on ESP32-C3; disabled
const int OLED_SDA_PIN = 6;
const int OLED_SCL_PIN = 7;

// === Global instances ===
Preferences preferences;
WebServer server(80);
ServoController servo(SERVO_PIN, 0, 180);
BatteryMonitor battery(BATTERY_PIN);
FeedingScheduler scheduler;
ApiHandlers apiHandlers(server, preferences, servo, battery, scheduler, LIGHT_PIN);
OledDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
DeviceRuntime deviceRuntime(apiHandlers, scheduler, servo, battery, oled);
PowerManager powerManager(apiHandlers, scheduler, servo, oled, BUTTON_PIN);
OtaHandler otaHandler(server, battery);

// === State variables ===
bool lastButtonState = HIGH;
uint32_t buttonPressStartMs = 0;

void performAutoFeeding(int repeats) {
  servo.feedSequenceAsync(repeats, []() {
    Serial.println("[SCHED] Auto feed complete");
  });
  powerManager.markInteraction();
}

void printStartupBanner() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n========================================");
  Serial.println("=== AquaFeed System Starting ===");
  Serial.printf("=== v%s  Built: %s %s ===\n", AQUAFEED_FIRMWARE_VERSION, BUILD_DATE, BUILD_TIME);
  Serial.println("========================================\n");
  Serial.flush();
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d ", reason);
  switch (reason) {
    case ESP_RST_BROWNOUT: Serial.println("(BROWNOUT - low voltage!)"); break;
    case ESP_RST_PANIC:    Serial.println("(PANIC/CRASH)"); break;
    case ESP_RST_WDT:      Serial.println("(WATCHDOG)"); break;
    case ESP_RST_SW:       Serial.println("(Software restart)"); break;
    case ESP_RST_POWERON:  Serial.println("(Power on)"); break;
    default:               Serial.println("(other)"); break;
  }
}

void initializeHardwareModules() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonState = digitalRead(BUTTON_PIN);

  Serial.println("Initializing battery monitor...");
  battery.begin();
  Serial.println("Initializing servo...");
  servo.begin();
  Serial.println("Initializing OLED display...");
  oled.begin();
  Serial.println("OLED initialization completed");
  Serial.println("All modules initialized!");
}

void loadApiSettingsFromPreferences() {
  preferences.begin("feeder", false);

  servo.setSpeed(preferences.getFloat("speed", 20.0));
  apiHandlers.setFeedRepeats(preferences.getInt("feedRepeats", 1));
  apiHandlers.setPowerSaveMode(preferences.getBool("powerSaveMode", true));
  apiHandlers.setDisplayEnabled(preferences.getBool("displayEnabled", true));

  uint32_t dsIdle = preferences.getUInt("dsIdleSec", 300);
  if (dsIdle < 10) {
    dsIdle = 10;
  }
  if (dsIdle > 3600) {
    dsIdle = 3600;
  }
  apiHandlers.setDeepSleepIdleSec(static_cast<uint16_t>(dsIdle));

  uint32_t dispOff = preferences.getUInt("displayOffSec", 20);
  if (dispOff < 5) {
    dispOff = 5;
  }
  if (dispOff > 600) {
    dispOff = 600;
  }
  apiHandlers.setDisplayOffAfterSec(static_cast<uint16_t>(dispOff));

  float batteryCalibration = preferences.getFloat("battCal", 0.518f);
  battery.setCalibrationFactor(batteryCalibration);
}

void initializeRuntimeState() {
  scheduler.begin(preferences);
  powerManager.initialize();
  setCpuFrequencyMhz(160);
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
}

void initializeNetworkAndServer() {
  initWiFi(preferences);
  WiFi.setSleep(apiHandlers.getPowerSaveMode() && !isAPMode);
  apiHandlers.setupRoutes();
  otaHandler.registerRoutes();
  setupWiFiHandlers(server, preferences);
  server.begin();
  Serial.println("HTTP server started");
}

void printConnectionSummary() {
  Serial.println("\n========================================");
  if (!isAPMode && WiFi.status() == WL_CONNECTED) {
    Serial.print("Web interface: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    Serial.println("mDNS: http://fish.local/");
    ets_printf("[AquaFeed v%s] IP: %s\n", AQUAFEED_FIRMWARE_VERSION, WiFi.localIP().toString().c_str());
  } else {
    Serial.println("AP Mode - Connect to WiFi:");
    Serial.print("  SSID: ");
    Serial.println(apSSID);
    Serial.print("  Password: ");
    Serial.println(apPassword);
    Serial.print("  Web interface: http://");
    Serial.println(WiFi.softAPIP());
    ets_printf("[AquaFeed v%s] AP mode: %s\n", AQUAFEED_FIRMWARE_VERSION, WiFi.softAPIP().toString().c_str());
  }
  Serial.println("========================================\n");
}

void printMemoryStatus() {
  Serial.printf("=== Memory Status ===\n");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Min free heap (ever): %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("Loop stack HWM: %d bytes free\n", uxTaskGetStackHighWaterMark(NULL) * 4);
  Serial.printf("====================\n");
}

void handleWakeCause(esp_sleep_wakeup_cause_t wakeCause) {
  if (wakeCause == ESP_SLEEP_WAKEUP_GPIO) {
    ets_printf("[WAKE] Feed button -> waking display only, not feeding\n");
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Waking the device just to check on it is the common case; feeding on
      // this same press was surprising. Wake the display; a second short
      // press (now that the device/display are awake) triggers the feed.
      powerManager.markInteraction();
    }
  } else if (wakeCause == ESP_SLEEP_WAKEUP_TIMER) {
    ets_printf("[WAKE] Schedule timer\n");
  }
}

void handleManualButtonPress() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    buttonPressStartMs = millis();
  }

  if (lastButtonState == LOW && buttonState == HIGH) {
    uint32_t pressDuration = millis() - buttonPressStartMs;
    ets_printf("[BTN] Released after %ums\n", pressDuration);
    if (pressDuration >= 5000) {
      ets_printf("[BTN] Long press -> AP mode\n");
      powerManager.markInteraction();
      startAPMode();
    } else if (!apiHandlers.isDisplayAwake()) {
      ets_printf("[BTN] Short press -> display was off, waking it (not feeding)\n");
      powerManager.markInteraction();
    } else if (!servo.isMoving()) {
      ets_printf("[BTN] Short press -> manual feed\n");
      powerManager.markInteraction();
      deviceRuntime.tryRunFeedSequence(
        apiHandlers.getFeedRepeats(),
        "Manual button feed blocked: recently fed",
        true
      );
    } else {
      Serial.println("[BTN] Short press ignored — servo already moving");
    }
    buttonPressStartMs = 0;
  }

  lastButtonState = buttonState;
}

void updateWiFiPowerSaveMode() {
  static bool appliedPowerSaveMode = false;
  static bool initializedPowerSettings = false;

  bool currentPowerSaveMode = apiHandlers.getPowerSaveMode();
  if (!initializedPowerSettings || currentPowerSaveMode != appliedPowerSaveMode) {
    WiFi.setSleep(currentPowerSaveMode && !isAPMode);
    appliedPowerSaveMode = currentPowerSaveMode;
    initializedPowerSettings = true;
    powerManager.markInteraction();
  }
}

// ============================================================================
// ============================================================================
// ============================================================================


// === Setup ===
void setup(){
  printStartupBanner();
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();

  initializeHardwareModules();
  loadApiSettingsFromPreferences();
  initializeRuntimeState();
  initializeNetworkAndServer();
  printConnectionSummary();
  printMemoryStatus();
  handleWakeCause(wakeCause);
}

// === Loop ===
void loop(){
  static uint32_t lastLoopMs = 0;
  if (millis() - lastLoopMs < 20) return;
  lastLoopMs = millis();



  server.handleClient();
  updateWiFiProvisioning();
  recheckClaimStatusIfNeeded();
  handleManualButtonPress();
  updateWiFiPowerSaveMode();

  scheduler.loop(performAutoFeeding);

  battery.update();
  NextFeedInfo nextFeed = scheduler.computeNextFeed();
  bool currentPowerSaveMode = apiHandlers.getPowerSaveMode();
  DisplayData displayData = deviceRuntime.buildDisplayData(nextFeed, isAPMode);
  bool displayShouldBeAwake = powerManager.shouldDisplayStayAwake(currentPowerSaveMode);
  deviceRuntime.updateDisplay(displayData, displayShouldBeAwake);
  powerManager.updateSleepState(isAPMode, displayShouldBeAwake);
  yield();
}
