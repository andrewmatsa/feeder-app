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
#include "web_pages.h"
#include "oled_display.h"

// === Hardware pins ===
const int SERVO_PIN = 4;
const int BUTTON_PIN = 3;
const int BATTERY_PIN = 2; // ⚡ MH Electronic Voltage Sensor (VOUT)
const int OLED_SDA_PIN = 6;
const int OLED_SCL_PIN = 7;

// === Global instances ===
Preferences preferences;
WebServer server(80);
ServoController servo(SERVO_PIN, 0, 180);
BatteryMonitor battery(BATTERY_PIN);
FeedingScheduler scheduler;
ApiHandlers apiHandlers(server, preferences, servo, battery, scheduler);
OledDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);

// === State variables ===
bool lastButtonState = HIGH;
unsigned long lastInteractionMillis = 0;
bool powerSaveSleepArmed = false;

// === Power Management ===
const unsigned long DISPLAY_WAKE_GRACE_MS = 5000;

static unsigned long powerSaveIdleMs() {
  return static_cast<unsigned long>(apiHandlers.getDeepSleepIdleSec()) * 1000UL;
}
const uint64_t MIN_DEEP_SLEEP_US = 5000000ULL;
const long FEED_WAKE_MARGIN_SECONDS = 15;

void markInteraction() {
  lastInteractionMillis = millis();
  powerSaveSleepArmed = true;
}

long computePreciseSecondsUntilNextFeed(const NextFeedInfo& nextInfo) {
  if (nextInfo.targetHour < 0 || nextInfo.targetMinute < 0) {
    return -1;
  }

  time_t now = time(nullptr);
  struct tm localTime;
  time_t adjusted = now + FeedingScheduler::KIEV_UTC_OFFSET_SECONDS;
  if (!gmtime_r(&adjusted, &localTime) || localTime.tm_year + 1900 < 2020) {
    return static_cast<long>(nextInfo.minutesUntil) * 60L;
  }

  int currentTotalSeconds = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;
  int nextTotalSeconds = nextInfo.targetHour * 3600 + nextInfo.targetMinute * 60;
  int diffSeconds = nextTotalSeconds - currentTotalSeconds;
  if (diffSeconds <= 0) {
    diffSeconds += 24 * 3600;
  }
  return diffSeconds;
}

uint64_t computeDeepSleepWakeMicros() {
  NextFeedInfo nextInfo = scheduler.computeNextFeed();
  long secondsUntilNextFeed = computePreciseSecondsUntilNextFeed(nextInfo);
  if (secondsUntilNextFeed <= 0) {
    return 60000000ULL;
  }

  long wakeAfterSeconds = secondsUntilNextFeed - FEED_WAKE_MARGIN_SECONDS;
  if (wakeAfterSeconds < 5) {
    wakeAfterSeconds = 5;
  }

  return static_cast<uint64_t>(wakeAfterSeconds) * 1000000ULL;
}

void enterPowerSaveDeepSleep() {
  if (apiHandlers.getDisplayEnabled()) {
    oled.showDeepSleepNotice(true);
  } else {
    delay(100);
  }

  oled.setPowerSave(true);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);

  uint64_t wakeMicros = computeDeepSleepWakeMicros();
  if (wakeMicros < MIN_DEEP_SLEEP_US) {
    wakeMicros = MIN_DEEP_SLEEP_US;
  }
  esp_sleep_enable_timer_wakeup(wakeMicros);
  Serial.printf("Power save: deep sleep, GPIO + timer wake in %.1f s (scheduled feed)\n", wakeMicros / 1000000.0);
  Serial.flush();
  delay(50);
  esp_deep_sleep_start();
}

long computeSleepCountdownSeconds(bool recentlyActive, bool webClientActive) {
  if (!apiHandlers.getPowerSaveMode() || isAPMode || servo.isMoving()) {
    return -1;
  }

  const unsigned long idleMs = powerSaveIdleMs();
  unsigned long nowMs = millis();
  unsigned long idleRemainingMs =
    (nowMs - lastInteractionMillis >= idleMs) ? 0 : (idleMs - (nowMs - lastInteractionMillis));
  unsigned long webRemainingMs =
    (nowMs - apiHandlers.getLastClientActivityMillis() >= idleMs) ? 0 : (idleMs - (nowMs - apiHandlers.getLastClientActivityMillis()));

  if (recentlyActive && webClientActive) {
    return static_cast<long>(max(idleRemainingMs, webRemainingMs) / 1000UL);
  }
  if (recentlyActive) {
    return static_cast<long>(idleRemainingMs / 1000UL);
  }
  if (webClientActive) {
    return static_cast<long>(webRemainingMs / 1000UL);
  }

  return 0;
}

String computeSleepReason(bool currentPowerSaveMode, bool displayShouldBeAwake, bool recentlyActive, bool webClientActive) {
  if (!currentPowerSaveMode) return "power_save_off";
  if (isAPMode) return "ap_mode";
  if (servo.isMoving()) return "feeding";
  if (webClientActive) return "web_active";
  if (recentlyActive) return "recent_activity";
  if (displayShouldBeAwake) return "display_awake";
  return "ready";
}

void performAutoFeeding(int repeats) {
  servo.feedSequence(repeats);
  markInteraction();
}

// ============================================================================
// ============================================================================
// ============================================================================


// === Setup ===
void setup(){
  Serial.begin(115200);
  delay(500);
  Serial.flush();
  
  for(int i = 0; i < 5; i++) {
    Serial.println("\n\n\n");
    Serial.println("========================================");
    Serial.println("=== AquaFeed System Starting ===");
    Serial.println("========================================");
    Serial.printf("Boot attempt: %d\n", i+1);
    delay(100);
  }
  Serial.println("\nSystem initialized successfully!\n");
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonState = digitalRead(BUTTON_PIN);
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  
  Serial.println("Initializing battery monitor...");
  battery.begin();
  Serial.println("Initializing servo...");
  servo.begin();
  Serial.println("Initializing OLED display...");
  oled.begin();
  Serial.println("OLED initialization completed");
  Serial.println("All modules initialized!");
  
  preferences.begin("feeder", false);
  
  servo.setSpeed(preferences.getFloat("speed", 20.0));
  apiHandlers.setFeedRepeats(preferences.getInt("feedRepeats", 1));
  apiHandlers.setPowerSaveMode(preferences.getBool("powerSaveMode", true));
  apiHandlers.setDisplayEnabled(preferences.getBool("displayEnabled", true));
  {
    uint32_t dsIdle = preferences.getUInt("dsIdleSec", 60);
    if (dsIdle < 10) {
      dsIdle = 10;
    }
    if (dsIdle > 3600) {
      dsIdle = 3600;
    }
    apiHandlers.setDeepSleepIdleSec(static_cast<uint16_t>(dsIdle));
  }

  scheduler.begin(preferences);
  
  lastInteractionMillis = millis();
  powerSaveSleepArmed = true;
  
  // ============================================================================
  // ============================================================================
  setCpuFrequencyMhz(160);
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  
  initWiFi(preferences);
  WiFi.setSleep(apiHandlers.getPowerSaveMode() && !isAPMode);

  apiHandlers.setupRoutes();
  
  setupWiFiHandlers(server, preferences);
  
  server.begin();
  Serial.println("HTTP server started");
  
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
  
  Serial.printf("=== Memory Status ===\n");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Min free heap (ever): %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("====================\n");

  if (wakeCause == ESP_SLEEP_WAKEUP_GPIO) {
    Serial.println("Wake source: feed button");
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (scheduler.canFeedNow()) {
        servo.feedSequence(apiHandlers.getFeedRepeats());
        scheduler.recordManualFeed();
        markInteraction();
      } else {
        Serial.println("Wake-up button feed blocked: recently fed");
      }
    }
  } else if (wakeCause == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Wake source: schedule timer");
  }
}

// === Loop ===
void loop(){
  // ============================================================================
  // ============================================================================
  server.handleClient();
  
  bool buttonState = digitalRead(BUTTON_PIN);
  if(lastButtonState == HIGH && buttonState == LOW && !servo.isMoving()) {
    if (scheduler.canFeedNow()) {
      if (apiHandlers.getDisplayEnabled()) {
        DisplayData feedDisplayData = {};
        feedDisplayData.isFeeding = true;
        oled.setPowerSave(false);
        oled.update(feedDisplayData);
      }
      
      servo.feedSequence(apiHandlers.getFeedRepeats());
      scheduler.recordManualFeed();
      markInteraction();
    } else {
      Serial.println("Manual button feed blocked: recently fed");
    }
  }
  lastButtonState = buttonState;

  static bool appliedPowerSaveMode = false;
  static bool initializedPowerSettings = false;
  bool currentPowerSaveMode = apiHandlers.getPowerSaveMode();
  if (!initializedPowerSettings || currentPowerSaveMode != appliedPowerSaveMode) {
    WiFi.setSleep(currentPowerSaveMode && !isAPMode);
    appliedPowerSaveMode = currentPowerSaveMode;
    initializedPowerSettings = true;
    markInteraction();
  }

  scheduler.loop(performAutoFeeding);

  battery.update();
  NextFeedInfo nextFeed = scheduler.computeNextFeed();
  
  time_t now = time(nullptr);
  struct tm localTime;
  int currentHour = -1, currentMinute = -1;
  time_t adjusted = now + FeedingScheduler::KIEV_UTC_OFFSET_SECONDS;
  if (gmtime_r(&adjusted, &localTime) && localTime.tm_year + 1900 >= 2020) {
    currentHour = localTime.tm_hour;
    currentMinute = localTime.tm_min;
  }
  
  DisplayData displayData;
  displayData.batteryVoltage = battery.getVoltage();
  displayData.batteryPercent = battery.getPercent();
  displayData.currentHour = currentHour;
  displayData.currentMinute = currentMinute;
  displayData.nextFeedHour = nextFeed.targetHour;
  displayData.nextFeedMinute = nextFeed.targetMinute;
  displayData.minutesUntilNext = nextFeed.minutesUntil;
  displayData.scheduledFeedsCount = scheduler.getFeedTimesCount();
  displayData.isAPMode = isAPMode;
  displayData.wifiConnected = (WiFi.status() == WL_CONNECTED);
  displayData.isFeeding = servo.isMoving();
  displayData.wifiSSID = savedSSID;
  
  bool displayShouldBeAwake =
    apiHandlers.getDisplayEnabled() &&
    (!currentPowerSaveMode || servo.isMoving() || (millis() - lastInteractionMillis) < DISPLAY_WAKE_GRACE_MS);

  oled.setPowerSave(!displayShouldBeAwake);
  if (displayShouldBeAwake) {
    oled.update(displayData);
  }

  const unsigned long idleMsLoop = powerSaveIdleMs();
  bool recentlyActive = (millis() - lastInteractionMillis) < idleMsLoop;
  bool webClientActive = (millis() - apiHandlers.getLastClientActivityMillis()) < idleMsLoop;
  long sleepCountdownSeconds = computeSleepCountdownSeconds(recentlyActive, webClientActive);
  String sleepReason = computeSleepReason(currentPowerSaveMode, displayShouldBeAwake, recentlyActive, webClientActive);
  apiHandlers.setSleepStatus(sleepReason, sleepCountdownSeconds, displayShouldBeAwake);
  if (currentPowerSaveMode &&
      powerSaveSleepArmed &&
      !servo.isMoving() &&
      !isAPMode &&
      !recentlyActive &&
      !webClientActive) {
    enterPowerSaveDeepSleep();
  }
  
  yield();
}
