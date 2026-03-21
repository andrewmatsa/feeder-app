#ifndef API_HANDLERS_H
#define API_HANDLERS_H

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
  
  void setPowerSaveMode(bool enabled) { powerSaveMode = enabled; }
  bool getPowerSaveMode() const { return powerSaveMode; }
  void setDisplayEnabled(bool enabled) { displayEnabled = enabled; }
  bool getDisplayEnabled() const { return displayEnabled; }
  void setFeedRepeats(int repeats) { feedRepeats = repeats; }
  int getFeedRepeats() const { return feedRepeats; }
  
private:
  WebServer& server;
  Preferences& preferences;
  ServoController& servo;
  BatteryMonitor& battery;
  FeedingScheduler& scheduler;
  
  bool powerSaveMode;
  bool displayEnabled;
  int feedRepeats;
  
  String cachedStatusJson;
  unsigned long cachedStatusTime;
  static const unsigned long STATUS_CACHE_TTL = 500;
  
  void invalidateCache() {
    cachedStatusJson = "";
    cachedStatusTime = 0;
  }
  
  static int extractIntField(const String& obj, char fieldKey, int fallback);
  static bool isDigitChar(char c);
  String buildStatusJson();
  
};

#endif

