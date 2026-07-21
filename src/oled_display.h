#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <U8g2lib.h>
#include <Wire.h>
#include "battery_monitor.h"
#include "feeding_scheduler.h"
#include "servo_controller.h"

struct DisplayData {
  float batteryVoltage;
  float batteryPercent;
  int currentHour;
  int currentMinute;
  int nextFeedHour;
  int nextFeedMinute;
  int minutesUntilNext;
  int scheduledFeedsCount;
  bool isAPMode;
  bool wifiConnected;
  bool isFeeding;
  bool isCharging;
  int cooldownSeconds;
  char ip[16];
};

class OledDisplay {
public:
  OledDisplay(int sdaPin = 6, int sclPin = 7);
  void begin();
  void update(const DisplayData& data);
  void clear();
  void setPowerSave(bool enabled);
  void showSplash();
  /** Shown before deep sleep; ASCII only. */
  void showDeepSleepNotice(bool scheduleTimerWakeEnabled);
  
private:
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
  int sdaPin;
  int sclPin;
  unsigned long lastUpdateTime;
  bool isInitialized;
  bool isPowerSaveEnabled;
  static const unsigned long UPDATE_INTERVAL_MS = 1000;
  unsigned long feedingStartTime;
  static const unsigned long FEEDING_DISPLAY_DURATION_MS = 3000;
  
  bool checkDisplay();
  
  void drawSimpleScreen(const DisplayData& data);
  void drawFeedingScreen();
  void formatTime(char* buffer, int hour, int minute);
};

#endif

