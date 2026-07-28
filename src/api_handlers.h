#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include <WebServer.h>
#include <Preferences.h>
#include "servo_controller.h"
#include "battery_monitor.h"
#include "feeding_scheduler.h"
#include "wifi_manager.h"

extern const char* pageHome;
extern const char* pageInfo;

class ApiHandlers {
public:
  ApiHandlers(WebServer& server, Preferences& preferences,
              ServoController& servo, BatteryMonitor& battery,
              FeedingScheduler& scheduler, int lightPin = -1);
  
  void setupRoutes();

  void handleStatus();
  void handleSetAngle();
  void handleFeedNow();
  void handleSetSpeed();
  void handleSetRepeats();
  void handleSetFeedTimes();
  void handleSetPowerMode();
  void handleSetDisplayMode();
  void handleSetDisplayOff();
  void handleSetDeepSleep();
  void handleSetTimezone();
  void handleSetFeedInterval();
  void handleSetBatteryCalibration();
  
  void setPowerSaveMode(bool enabled) { powerSaveMode = enabled; }
  bool getPowerSaveMode() const { return powerSaveMode; }
  void setDisplayEnabled(bool enabled) { displayEnabled = enabled; }
  bool getDisplayEnabled() const { return displayEnabled; }
  void setDisplayOffAfterSec(uint16_t sec);
  uint16_t getDisplayOffAfterSec() const { return displayOffAfterSec; }
  void setDeepSleepIdleSec(uint16_t sec);
  uint16_t getDeepSleepIdleSec() const { return deepSleepIdleSec; }
  void setFeedRepeats(int repeats) { feedRepeats = repeats; }
  int getFeedRepeats() const { return feedRepeats; }
  unsigned long getLastClientActivityMillis() const { return lastClientActivityMillis; }
  bool isDisplayAwake() const { return displayAwakeNow; }
  String getWifiIp() const;
  void setSleepStatus(const String& reason, long countdownSeconds, bool displayAwake) {
    sleepReason = reason;
    sleepCountdownSeconds = countdownSeconds;
    displayAwakeNow = displayAwake;
  }
  
private:
  WebServer& server;
  Preferences& preferences;
  ServoController& servo;
  BatteryMonitor& battery;
  FeedingScheduler& scheduler;
  int lightPin;

  bool powerSaveMode;
  bool displayEnabled;
  uint16_t displayOffAfterSec;
  uint16_t deepSleepIdleSec;
  int feedRepeats;
  
  String cachedStatusJson;
  unsigned long cachedStatusTime;
  unsigned long lastClientActivityMillis;
  String sleepReason;
  long sleepCountdownSeconds;
  bool displayAwakeNow;
  static const unsigned long STATUS_CACHE_TTL = 500;
  
  void invalidateCache() {
    cachedStatusJson = "";
    cachedStatusTime = 0;
  }
  void noteClientActivity() { lastClientActivityMillis = millis(); }
  
  String buildStatusJson();
  void populateStatusDocument(JsonDocument& doc);
  void appendFeedTimes(JsonArray feedTimesArray) const;
  void appendRuntimeStatus(JsonDocument& doc, const NextFeedInfo& nextFeed) const;
  void appendMemoryStatus(JsonDocument& doc) const;
  String getCurrentTimeString() const;
  unsigned long computeCacheAge(bool hasCachedStatus) const;
  int parseFeedTimesJson(const String& jsonData, FeedTime* target, int maxCount) const;
  void saveFeedTimes(const FeedTime* newFeedTimes, int count);
  
};

#endif

