#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <Arduino.h>
#include <stdint.h>
#include <WebServer.h>
#include <Preferences.h>
#include "servo_controller.h"
#include "battery_monitor.h"
#include "feeding_scheduler.h"
#include "wifi_manager.h"

class ApiHandlers {
public:
  ApiHandlers(WebServer& server, Preferences& preferences,
              ServoController& servo, BatteryMonitor& battery,
              FeedingScheduler& scheduler);
  
  void setupRoutes();
  
  void handleRoot();
  void handleInfo();
  
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
  
  static int extractIntField(const String& obj, char fieldKey, int fallback);
  static bool isDigitChar(char c);
  String buildStatusJson();
  
};

#endif

