#include "power_manager.h"

#include "time.h"
#include "esp_sleep.h"

namespace {
constexpr uint64_t MIN_DEEP_SLEEP_US = 5000000ULL;
constexpr long FEED_WAKE_MARGIN_SECONDS = 15;
constexpr long MIN_WORTHWHILE_DEEP_SLEEP_SECONDS = 120;
}

PowerManager::PowerManager(ApiHandlers& apiHandlers,
                           FeedingScheduler& scheduler,
                           ServoController& servo,
                           OledDisplay& oled,
                           int buttonPin)
  : apiHandlers(apiHandlers),
    scheduler(scheduler),
    servo(servo),
    oled(oled),
    buttonPin(buttonPin),
    lastInteractionMillis(0),
    powerSaveSleepArmed(false) {
}

void PowerManager::initialize() {
  lastInteractionMillis = millis();
  powerSaveSleepArmed = true;
}

void PowerManager::markInteraction() {
  lastInteractionMillis = millis();
  powerSaveSleepArmed = true;
}

unsigned long PowerManager::powerSaveIdleMs() const {
  return static_cast<unsigned long>(apiHandlers.getDeepSleepIdleSec()) * 1000UL;
}

long PowerManager::computePreciseSecondsUntilNextFeed(const NextFeedInfo& nextInfo) const {
  if (nextInfo.targetHour < 0 || nextInfo.targetMinute < 0) {
    return -1;
  }

  time_t now = time(nullptr);
  struct tm localTime;
  if (!FeedingScheduler::toLocalTime(now, localTime) || localTime.tm_year + 1900 < 2020) {
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

long PowerManager::computeSecondsUntilNextFeed() const {
  return computePreciseSecondsUntilNextFeed(scheduler.computeNextFeed());
}

bool PowerManager::isDeepSleepWorthwhile(long secondsUntilNextFeed) const {
  if (secondsUntilNextFeed <= 0) {
    return true;
  }
  return (secondsUntilNextFeed - FEED_WAKE_MARGIN_SECONDS) >= MIN_WORTHWHILE_DEEP_SLEEP_SECONDS;
}

uint64_t PowerManager::computeDeepSleepWakeMicros() const {
  long secondsUntilNextFeed = computeSecondsUntilNextFeed();
  if (secondsUntilNextFeed <= 0) {
    long fallbackSleepSeconds = apiHandlers.getDeepSleepIdleSec();
    if (fallbackSleepSeconds < 900) {
      fallbackSleepSeconds = 900;
    }
    return static_cast<uint64_t>(fallbackSleepSeconds) * 1000000ULL;
  }

  long wakeAfterSeconds = secondsUntilNextFeed - FEED_WAKE_MARGIN_SECONDS;
  if (wakeAfterSeconds < 5) {
    wakeAfterSeconds = 5;
  }

  return static_cast<uint64_t>(wakeAfterSeconds) * 1000000ULL;
}

void PowerManager::applyCpuPowerProfile(bool lowPowerMode) const {
  const int targetFreq = lowPowerMode ? LOW_POWER_CPU_FREQUENCY_MHZ : ACTIVE_CPU_FREQUENCY_MHZ;
  if (getCpuFrequencyMhz() != targetFreq) {
    setCpuFrequencyMhz(targetFreq);
  }
}

void PowerManager::enterPowerSaveDeepSleep() {
  if (apiHandlers.getDisplayEnabled()) {
    oled.showDeepSleepNotice(true);
  } else {
    delay(100);
  }

  servo.setPowerSave(true);
  oled.setPowerSave(true);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << buttonPin, ESP_GPIO_WAKEUP_GPIO_LOW);

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

long PowerManager::computeSleepCountdownSeconds(bool isAPMode, bool recentlyActive, bool webClientActive) const {
  if (!apiHandlers.getPowerSaveMode() || isAPMode || servo.isMoving()) {
    return -1;
  }

  if (!isDeepSleepWorthwhile(computeSecondsUntilNextFeed())) {
    return -1;
  }

  const unsigned long idleMs = powerSaveIdleMs();
  unsigned long nowMs = millis();
  unsigned long idleRemainingMs =
    (nowMs - lastInteractionMillis >= idleMs) ? 0 : (idleMs - (nowMs - lastInteractionMillis));
  unsigned long webRemainingMs =
    (nowMs - apiHandlers.getLastClientActivityMillis() >= idleMs)
      ? 0
      : (idleMs - (nowMs - apiHandlers.getLastClientActivityMillis()));

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

String PowerManager::computeSleepReason(bool isAPMode,
                                        bool currentPowerSaveMode,
                                        bool displayShouldBeAwake,
                                        bool recentlyActive,
                                        bool webClientActive) const {
  if (!currentPowerSaveMode) return "power_save_off";
  if (isAPMode) return "ap_mode";
  if (servo.isMoving()) return "feeding";
  if (webClientActive) return "web_active";
  if (recentlyActive) return "recent_activity";
  if (displayShouldBeAwake) return "display_awake";
  if (!isDeepSleepWorthwhile(computeSecondsUntilNextFeed())) return "next_feed_soon";
  return "ready";
}

bool PowerManager::shouldDisplayStayAwake(bool currentPowerSaveMode) const {
  const unsigned long displayGraceMs =
    static_cast<unsigned long>(apiHandlers.getDisplayOffAfterSec()) * 1000UL;
  return apiHandlers.getDisplayEnabled() &&
    (!currentPowerSaveMode || servo.isMoving() || (millis() - lastInteractionMillis) < displayGraceMs);
}

void PowerManager::updateSleepState(bool isAPMode, bool displayShouldBeAwake) {
  bool currentPowerSaveMode = apiHandlers.getPowerSaveMode();
  const unsigned long idleMsLoop = powerSaveIdleMs();
  bool recentlyActive = (millis() - lastInteractionMillis) < idleMsLoop;
  bool webClientActive = (millis() - apiHandlers.getLastClientActivityMillis()) < idleMsLoop;
  bool lowPowerCpuMode = currentPowerSaveMode && !isAPMode && !servo.isMoving() && !displayShouldBeAwake;
  bool lowPowerPeripheralMode = lowPowerCpuMode && !displayShouldBeAwake;
  bool deepSleepWorthwhile = isDeepSleepWorthwhile(computeSecondsUntilNextFeed());

  servo.setPowerSave(lowPowerPeripheralMode);
  applyCpuPowerProfile(lowPowerCpuMode);

  long sleepCountdownSeconds = computeSleepCountdownSeconds(isAPMode, recentlyActive, webClientActive);
  String sleepReason = computeSleepReason(
    isAPMode,
    currentPowerSaveMode,
    displayShouldBeAwake,
    recentlyActive,
    webClientActive
  );

  apiHandlers.setSleepStatus(sleepReason, sleepCountdownSeconds, displayShouldBeAwake);
  if (currentPowerSaveMode &&
      powerSaveSleepArmed &&
      !servo.isMoving() &&
      !isAPMode &&
      deepSleepWorthwhile &&
      !recentlyActive &&
      !webClientActive) {
    enterPowerSaveDeepSleep();
  }
}
