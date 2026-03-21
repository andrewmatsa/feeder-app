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
unsigned long lastAutoFeedMillis = 0;
bool autoFeedSleepPending = false;

// === Power Management ===
const unsigned long SLEEP_INTERVAL = 60000;

void enterLightSleep() {
  Serial.println("Entering light sleep for power saving...");
  esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL * 1000);
  if (esp_light_sleep_start() == ESP_OK) {
    Serial.println("Woke up from sleep");
  }
}

void performAutoFeeding(int repeats) {
  servo.feedSequence(repeats);
  if (apiHandlers.getPowerSaveMode()) {
    lastAutoFeedMillis = millis();
    autoFeedSleepPending = true;
  }
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
  
  scheduler.begin(preferences);
  
  autoFeedSleepPending = false;
  lastAutoFeedMillis = 0;
  
  // ============================================================================
  // ============================================================================
  setCpuFrequencyMhz(160);
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  
  initWiFi(preferences);

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
}

// === Loop ===
void loop(){
  // ============================================================================
  // ============================================================================
  server.handleClient();
  
  bool buttonState = digitalRead(BUTTON_PIN);
  if(lastButtonState == HIGH && buttonState == LOW && !servo.isMoving()) {
    if (apiHandlers.getDisplayEnabled()) {
      DisplayData feedDisplayData = {};
      feedDisplayData.isFeeding = true;
      oled.update(feedDisplayData);
    }
    
    servo.feedSequence(apiHandlers.getFeedRepeats());
    scheduler.recordManualFeed();
  }
  lastButtonState = buttonState;

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
  
  if (apiHandlers.getDisplayEnabled()) {
    oled.update(displayData);
  } else {
    static unsigned long lastClearTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastClearTime > 1000) {
      oled.clear();
      lastClearTime = currentTime;
    }
  }

  if (apiHandlers.getPowerSaveMode() && autoFeedSleepPending && !servo.isMoving() && !isAPMode) {
    unsigned long elapsed = millis() - lastAutoFeedMillis;
    if (elapsed >= 60000UL) {
      NextFeedInfo nextInfo = scheduler.computeNextFeed();
      if (nextInfo.minutesUntil > 0) {
        long secondsUntil = static_cast<long>(nextInfo.minutesUntil) * 60L;
        if (secondsUntil > 60) {
          Serial.printf("Power save: entering light sleep for up to %ld seconds (next feed in %ld seconds)\n",
                        secondsUntil - 30, secondsUntil);
          autoFeedSleepPending = false;
          uint64_t wakeMicros = (secondsUntil - 30) * 1000000ULL;
          if (wakeMicros < 30000000ULL) wakeMicros = 30000000ULL;
          esp_sleep_enable_timer_wakeup(wakeMicros);
          enterLightSleep();
        } else {
          autoFeedSleepPending = false;
        }
      } else {
        autoFeedSleepPending = false;
      }
    }
  }
  
  yield();
}
