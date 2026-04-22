#ifndef DEVICE_RUNTIME_H
#define DEVICE_RUNTIME_H

#include <Arduino.h>
#include "api_handlers.h"
#include "battery_monitor.h"
#include "feeding_scheduler.h"
#include "oled_display.h"
#include "servo_controller.h"

class DeviceRuntime {
public:
  DeviceRuntime(ApiHandlers& apiHandlers,
                FeedingScheduler& scheduler,
                ServoController& servo,
                BatteryMonitor& battery,
                OledDisplay& oled);

  bool tryRunFeedSequence(int repeats, const char* blockedMessage, bool showDisplay);
  DisplayData buildDisplayData(const NextFeedInfo& nextFeed, bool isAPMode, const String& wifiSSID) const;
  void updateDisplay(const DisplayData& displayData, bool displayShouldBeAwake);

private:
  ApiHandlers& apiHandlers;
  FeedingScheduler& scheduler;
  ServoController& servo;
  BatteryMonitor& battery;
  OledDisplay& oled;

  void getCurrentLocalTime(int& currentHour, int& currentMinute) const;
};

#endif
