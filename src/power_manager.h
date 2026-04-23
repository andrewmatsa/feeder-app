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
  static const int ACTIVE_CPU_FREQUENCY_MHZ = 160;
  static const int LOW_POWER_CPU_FREQUENCY_MHZ = 80;

  ApiHandlers& apiHandlers;
  FeedingScheduler& scheduler;
  ServoController& servo;
  OledDisplay& oled;
  int buttonPin;
  unsigned long lastInteractionMillis;
  bool powerSaveSleepArmed;

  unsigned long powerSaveIdleMs() const;
  long computePreciseSecondsUntilNextFeed(const NextFeedInfo& nextInfo) const;
  long computeSecondsUntilNextFeed() const;
  uint64_t computeDeepSleepWakeMicros() const;
  bool isDeepSleepWorthwhile(long secondsUntilNextFeed) const;
  void applyCpuPowerProfile(bool lowPowerMode) const;
  void enterPowerSaveDeepSleep();
  long computeSleepCountdownSeconds(bool isAPMode, bool recentlyActive, bool webClientActive) const;
  String computeSleepReason(bool isAPMode,
                            bool currentPowerSaveMode,
                            bool displayShouldBeAwake,
                            bool recentlyActive,
                            bool webClientActive) const;
};

#endif
