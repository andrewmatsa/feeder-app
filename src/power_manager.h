#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <stdint.h>
#include "api_handlers.h"
#include "feeding_scheduler.h"
#include "servo_controller.h"
#include "oled_display.h"

class PowerManager {
public:
  PowerManager(ApiHandlers& apiHandlers,
               FeedingScheduler& scheduler,
               ServoController& servo,
               OledDisplay& oled,
               int buttonPin);

  void initialize();
  void markInteraction();
  bool shouldDisplayStayAwake(bool currentPowerSaveMode) const;
  void updateSleepState(bool isAPMode, bool displayShouldBeAwake);

private:
  ApiHandlers& apiHandlers;
  FeedingScheduler& scheduler;
  ServoController& servo;
  OledDisplay& oled;
  int buttonPin;
  unsigned long lastInteractionMillis;
  bool powerSaveSleepArmed;

  unsigned long powerSaveIdleMs() const;
  long computePreciseSecondsUntilNextFeed(const NextFeedInfo& nextInfo) const;
  uint64_t computeDeepSleepWakeMicros() const;
  void enterPowerSaveDeepSleep();
  long computeSleepCountdownSeconds(bool isAPMode, bool recentlyActive, bool webClientActive) const;
  String computeSleepReason(bool isAPMode,
                            bool currentPowerSaveMode,
                            bool displayShouldBeAwake,
                            bool recentlyActive,
                            bool webClientActive) const;
};

#endif
