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

// === Hardware pins ===
const int SERVO_PIN = 4;
const int BUTTON_PIN = 3;
const int BATTERY_PIN = 2; // ⚡ MH Electronic Voltage Sensor (VOUT)
const int LIGHT_PIN = 5;   // LDR voltage divider: 3.3V → LDR → GPIO5 → 10kΩ → GND
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
  Serial.println("\n========================================");
  Serial.println("=== AquaFeed System Starting ===");
  Serial.println("========================================\n");
}

void initializeHardwareModules() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonState = digitalRead(BUTTON_PIN);
  pinMode(LIGHT_PIN, INPUT);

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
  } else {
    Serial.println("AP Mode - Connect to WiFi:");
    Serial.print("  SSID: ");
    Serial.println(apSSID);
    Serial.print("  Password: ");
    Serial.println(apPassword);
    Serial.print("  Web interface: http://");
    Serial.println(WiFi.softAPIP());
  }
  Serial.println("========================================\n");
}

void printMemoryStatus() {
  Serial.printf("=== Memory Status ===\n");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Min free heap (ever): %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("====================\n");
}

void handleWakeCause(esp_sleep_wakeup_cause_t wakeCause) {
  if (wakeCause == ESP_SLEEP_WAKEUP_GPIO) {
    Serial.println("Wake source: feed button");
    if (digitalRead(BUTTON_PIN) == LOW) {
      powerManager.markInteraction();
      deviceRuntime.tryRunFeedSequence(
        apiHandlers.getFeedRepeats(),
        "Wake-up button feed blocked: recently fed",
        false
      );
    }
  } else if (wakeCause == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Wake source: schedule timer");
  }
}

void handleManualButtonPress() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    buttonPressStartMs = millis();
  }

  if (lastButtonState == LOW && buttonState == HIGH) {
    uint32_t pressDuration = millis() - buttonPressStartMs;
    Serial.printf("[BTN] Released after %ums\n", pressDuration);
    if (pressDuration >= 5000) {
      Serial.println("[BTN] Long press → entering AP mode");
      powerManager.markInteraction();
      startAPMode();
    } else if (!servo.isMoving()) {
      Serial.println("[BTN] Short press → manual feed");
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

  static uint32_t lastStatusLogMs = 0;
  if (millis() - lastStatusLogMs > 30000) {
    lastStatusLogMs = millis();
    Serial.printf("[STATUS] Battery: %.2fV (%d%%) | WiFi: %s | IP: %s\n",
      battery.getVoltage(), battery.getPercent(),
      WiFi.status() == WL_CONNECTED ? WiFi.SSID().c_str() : "disconnected",
      isAPMode ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str()
    );
  }

  server.handleClient();
  updateWiFiProvisioning();
  handleManualButtonPress();
  updateWiFiPowerSaveMode();

  scheduler.loop(performAutoFeeding);

  battery.update();
  NextFeedInfo nextFeed = scheduler.computeNextFeed();
  bool currentPowerSaveMode = apiHandlers.getPowerSaveMode();
  DisplayData displayData = deviceRuntime.buildDisplayData(nextFeed, isAPMode, savedSSID);
  bool displayShouldBeAwake = powerManager.shouldDisplayStayAwake(currentPowerSaveMode);
  deviceRuntime.updateDisplay(displayData, displayShouldBeAwake);
  powerManager.updateSleepState(isAPMode, displayShouldBeAwake);
  yield();
}
