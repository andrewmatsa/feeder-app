#include "device_runtime.h"

#include "time.h"
#include "wifi_manager.h"

DeviceRuntime::DeviceRuntime(ApiHandlers& apiHandlers,
                             FeedingScheduler& scheduler,
                             ServoController& servo,
                             BatteryMonitor& battery,
                             OledDisplay& oled)
  : apiHandlers(apiHandlers),
    scheduler(scheduler),
    servo(servo),
    battery(battery),
    oled(oled) {
}

bool DeviceRuntime::tryRunFeedSequence(int repeats, const char* blockedMessage, bool showDisplay) {
  if (!scheduler.canFeedNow()) {
    ets_printf("[FEED] Blocked: %s\n", blockedMessage);
    return false;
  }

  if (showDisplay && apiHandlers.getDisplayEnabled()) {
    DisplayData feedDisplayData = {};
    feedDisplayData.isFeeding = true;
    oled.setPowerSave(false);
    oled.update(feedDisplayData);
  }

  ets_printf("[FEED] Button feed: %d repeat(s)\n", repeats);
  servo.feedSequenceAsync(repeats, [this]() {
    Serial.println("[FEED] Button feed complete");
    scheduler.recordManualFeed();
  });
  return true;
}

void DeviceRuntime::getCurrentLocalTime(int& currentHour, int& currentMinute) const {
  currentHour = -1;
  currentMinute = -1;

  time_t now = time(nullptr);
  struct tm localTime;
  if (FeedingScheduler::toLocalTime(now, localTime) && localTime.tm_year + 1900 >= 2020) {
    currentHour = localTime.tm_hour;
    currentMinute = localTime.tm_min;
  }
}

DisplayData DeviceRuntime::buildDisplayData(const NextFeedInfo& nextFeed,
                                            bool isAPMode) const {
  int currentHour;
  int currentMinute;
  getCurrentLocalTime(currentHour, currentMinute);

  DisplayData displayData;
  displayData.ip[0] = '\0';
  if (!isAPMode && WiFi.status() == WL_CONNECTED) {
    apiHandlers.getWifiIp().toCharArray(displayData.ip, sizeof(displayData.ip));
  }
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
  displayData.isCharging = battery.isCharging();
  displayData.cooldownSeconds = scheduler.getSecondsUntilManualFeedAllowed();
  displayData.deviceClaimed = isDeviceClaimed;
  displayData.apSsid[0] = '\0';
  displayData.provisionState[0] = '\0';
  displayData.provisionSsid[0] = '\0';
  if (isAPMode) {
    apSSID.toCharArray(displayData.apSsid, sizeof(displayData.apSsid));
    strncpy(displayData.provisionState, getProvisionStateStr(), sizeof(displayData.provisionState) - 1);
    displayData.provisionState[sizeof(displayData.provisionState) - 1] = '\0';
    getProvisionTargetSsid().toCharArray(displayData.provisionSsid, sizeof(displayData.provisionSsid));
  }
  return displayData;
}

void DeviceRuntime::updateDisplay(const DisplayData& displayData, bool displayShouldBeAwake) {
  oled.setPowerSave(!displayShouldBeAwake);
  if (displayShouldBeAwake) {
    oled.update(displayData);
  }
}
