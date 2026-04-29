#include "api_handlers.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>
#include "wifi_manager.h"
#include "web_pages.h"

ApiHandlers::ApiHandlers(WebServer& server, Preferences& preferences,
                         ServoController& servo, BatteryMonitor& battery,
                         FeedingScheduler& scheduler)
  : server(server), preferences(preferences),
    servo(servo), battery(battery), scheduler(scheduler),
    powerSaveMode(true), displayEnabled(true),
    displayOffAfterSec(20),
    deepSleepIdleSec(300),
    feedRepeats(1),
    cachedStatusTime(0), lastClientActivityMillis(0),
    sleepReason("unknown"), sleepCountdownSeconds(-1), displayAwakeNow(true) {
}

void ApiHandlers::setDeepSleepIdleSec(uint16_t sec) {
  if (sec < 10) {
    sec = 10;
  }
  if (sec > 3600) {
    sec = 3600;
  }
  deepSleepIdleSec = sec;
}

void ApiHandlers::setDisplayOffAfterSec(uint16_t sec) {
  if (sec < 5) {
    sec = 5;
  }
  if (sec > 600) {
    sec = 600;
  }
  displayOffAfterSec = sec;
}

void ApiHandlers::setupRoutes() {
  configureRequestSecurity(server);
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/info", HTTP_GET, [this]() { handleInfo(); });
  server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server.on("/api/setAngle", HTTP_POST, [this]() { handleSetAngle(); });
  server.on("/api/feedNow", HTTP_POST, [this]() { handleFeedNow(); });
  server.on("/api/setSpeed", HTTP_POST, [this]() { handleSetSpeed(); });
  server.on("/api/setRepeats", HTTP_POST, [this]() { handleSetRepeats(); });
  server.on("/api/setFeedTimes", HTTP_POST, [this]() { handleSetFeedTimes(); });
  server.on("/api/setPowerMode", HTTP_POST, [this]() { handleSetPowerMode(); });
  server.on("/api/setDisplayMode", HTTP_POST, [this]() { handleSetDisplayMode(); });
  server.on("/api/setDisplayOff", HTTP_POST, [this]() { handleSetDisplayOff(); });
  server.on("/api/setDeepSleep", HTTP_POST, [this]() { handleSetDeepSleep(); });
  server.on("/api/setTimezone", HTTP_POST, [this]() { handleSetTimezone(); });
  server.on("/api/setFeedInterval", HTTP_POST, [this]() { handleSetFeedInterval(); });
  server.on("/api/setBatteryCalibration", HTTP_POST, [this]() { handleSetBatteryCalibration(); });
}

void ApiHandlers::handleRoot() {
  noteClientActivity();
  if (isAPMode) {
    handleWiFi(server);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(200, "text/html", pageIndex);
}

void ApiHandlers::handleInfo() {
  noteClientActivity();
  if (isAPMode || !isApSessionAuthorized(server)) {
    handleWiFi(server);
    return;
  }
  server.send(200, "text/html", pageInfo);
}

void ApiHandlers::appendFeedTimes(JsonArray feedTimesArray) const {
  const FeedTime* feedTimes = scheduler.getFeedTimes();
  int feedTimesCount = scheduler.getFeedTimesCount();
  for (int i = 0; i < feedTimesCount; i++) {
    JsonObject feedTime = feedTimesArray.add<JsonObject>();
    feedTime["h"] = feedTimes[i].hour;
    feedTime["m"] = feedTimes[i].minute;
    feedTime["r"] = feedTimes[i].repeats;
  }
}

void ApiHandlers::appendLegacyFeedTimes(JsonDocument& doc) const {
  int h1, m1, r1, h2, m2, r2;
  scheduler.getLegacyFeedTimes(h1, m1, r1, h2, m2, r2);
  doc["feedHour1"] = h1;
  doc["feedMinute1"] = m1;
  doc["feedHour2"] = h2;
  doc["feedMinute2"] = m2;
  doc["feedRepeats1"] = r1;
  doc["feedRepeats2"] = r2;
}

String ApiHandlers::getCurrentTimeString() const {
  time_t now = time(nullptr);
  struct tm localTime;
  char timeBuf[6] = "--:--";
  if (FeedingScheduler::toLocalTime(now, localTime) && localTime.tm_year + 1900 >= 2020) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", localTime.tm_hour, localTime.tm_min);
  }
  return String(timeBuf);
}

String ApiHandlers::getWifiIp() const {
  if (!isAPMode && WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return String("");
}

unsigned long ApiHandlers::computeCacheAge(bool hasCachedStatus) const {
  if (!hasCachedStatus || cachedStatusTime == 0) {
    return 0;
  }
  return millis() - cachedStatusTime;
}

void ApiHandlers::appendRuntimeStatus(JsonDocument& doc, const NextFeedInfo& nextFeed) const {
  const bool hasCachedStatus = cachedStatusJson.length() > 0;
  doc["nextFeedMinutes"] = nextFeed.minutesUntil;
  doc["nextFeedHour"] = nextFeed.targetHour;
  doc["nextFeedMinute"] = nextFeed.targetMinute;
  doc["currentTime"] = getCurrentTimeString();
  doc["wifiSSID"] = savedSSID;
  doc["isAPMode"] = isAPMode;
  doc["sleepReason"] = sleepReason;
  doc["sleepCountdownSeconds"] = sleepCountdownSeconds;
  doc["displayAwake"] = displayAwakeNow;
  doc["manualFeedCooldownSeconds"] = scheduler.getSecondsUntilManualFeedAllowed();
  doc["minFeedIntervalMin"] = scheduler.getMinFeedIntervalMinutes();
  doc["timezoneOffsetMin"] = FeedingScheduler::getTimezoneOffsetMinutes();
  doc["wifiIP"] = getWifiIp();
  doc["cacheSize"] = cachedStatusJson.length();
  doc["cacheAge"] = computeCacheAge(hasCachedStatus);
  doc["cacheValid"] = hasCachedStatus;
}

void ApiHandlers::appendMemoryStatus(JsonDocument& doc) const {
  unsigned long freeHeap = ESP.getFreeHeap();
  unsigned long maxAllocHeap = ESP.getMaxAllocHeap();
  unsigned long minFreeHeap = ESP.getMinFreeHeap();
  unsigned long totalHeap = ESP.getHeapSize();

  doc["memoryFreeHeap"] = freeHeap;
  doc["memoryMaxAllocHeap"] = maxAllocHeap;
  doc["memoryMinFreeHeap"] = minFreeHeap;
  doc["memoryTotalHeap"] = totalHeap;
  doc["memoryUsedHeap"] = (totalHeap - freeHeap);
  doc["cpuFrequency"] = getCpuFrequencyMhz();
}

void ApiHandlers::populateStatusDocument(JsonDocument& doc) {
  battery.update();

  NextFeedInfo nextFeed = scheduler.computeNextFeed();
  doc["status"] = "ok";
  doc["currentAngle"] = servo.getCurrentAngle();
  doc["speed"] = servo.getSpeed();
  doc["feedRepeats"] = feedRepeats;
  doc["powerSaveMode"] = powerSaveMode;
  doc["displayEnabled"] = displayEnabled;
  doc["displayOffAfterSec"] = displayOffAfterSec;
  doc["deepSleepIdleSec"] = deepSleepIdleSec;
  doc["deepSleepWakeButtonOnly"] = false;
  doc["batteryVoltage"] = battery.getVoltage();
  doc["batteryPercent"] = static_cast<int>(battery.getPercent());
  doc["batteryCalibration"] = battery.getCalibrationFactor();
  doc["isCharging"] = battery.isCharging();

  appendFeedTimes(doc["feedTimes"].to<JsonArray>());
  appendLegacyFeedTimes(doc);
  appendRuntimeStatus(doc, nextFeed);
  appendMemoryStatus(doc);
}

String ApiHandlers::buildStatusJson() {
  JsonDocument doc;
  populateStatusDocument(doc);

  String json;
  json.reserve(900);
  serializeJson(doc, json);
  return json;
}

void ApiHandlers::handleStatus() {
  noteClientActivity();
  if (isAPMode || !isApSessionAuthorized(server)) {
    server.send(403, "text/plain", "login required");
    return;
  }
  // ============================================================================
  // ============================================================================
  // ============================================================================
  unsigned long now = millis();
  if (cachedStatusJson.length() == 0 || (now - cachedStatusTime) > STATUS_CACHE_TTL) {
    cachedStatusJson = buildStatusJson();
    cachedStatusTime = now;
  }
  server.send(200, "application/json", cachedStatusJson);
}

void ApiHandlers::handleSetAngle() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if(server.hasArg("angle") && !servo.isMoving()) {
    servo.setAngle(server.arg("angle").toInt(), false);
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleFeedNow() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  int repeatsForFeed = feedRepeats;
  if (server.hasArg("repeats")) {
    repeatsForFeed = constrain(server.arg("repeats").toInt(), 1, 20);
  }
  if (!scheduler.canFeedNow()) {
    server.send(429, "text/plain", "feeding blocked: recently fed");
    return;
  }

  server.send(200, "text/plain", "feeding");
  delay(10);

  servo.feedSequence(repeatsForFeed);
  scheduler.recordManualFeed();
  invalidateCache();
}

void ApiHandlers::handleSetSpeed() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if(server.hasArg("speed")) {
    float speed = server.arg("speed").toFloat();
    servo.setSpeed(speed);
    preferences.putFloat("speed", speed);
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetRepeats() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if(server.hasArg("repeats")) {
    feedRepeats = server.arg("repeats").toInt();
    preferences.putInt("feedRepeats", feedRepeats);
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

int ApiHandlers::parseFeedTimesJson(const String& jsonData, FeedTime* target, int maxCount) const {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  if (error || !doc.is<JsonArray>()) {
    return 0;
  }

  JsonArray feedTimes = doc.as<JsonArray>();
  int count = 0;
  for (JsonObjectConst item : feedTimes) {
    if (count >= maxCount) {
      break;
    }
    JsonVariantConst hourValue = item["h"].isNull() ? item["hour"] : item["h"];
    JsonVariantConst minuteValue = item["m"].isNull() ? item["minute"] : item["m"];
    JsonVariantConst repeatsValue = item["r"].isNull() ? item["repeats"] : item["r"];

    int h = constrain(hourValue.isNull() ? 10 : hourValue.as<int>(), 0, 23);
    int m = constrain(minuteValue.isNull() ? 0 : minuteValue.as<int>(), 0, 59);
    int r = max(1, repeatsValue.isNull() ? 1 : repeatsValue.as<int>());
    target[count++] = {h, m, r, false};
  }
  return count;
}

void ApiHandlers::saveFeedTimes(const FeedTime* newFeedTimes, int count) {
  scheduler.setFeedTimes(newFeedTimes, count);
  scheduler.saveToPreferences(preferences);
  invalidateCache();
  Serial.printf("Feed times saved: count=%d\n", count);
  for (int i = 0; i < count; i++) {
    Serial.printf("  [%d] %02d:%02d (repeats=%d)\n",
                  i,
                  newFeedTimes[i].hour,
                  newFeedTimes[i].minute,
                  newFeedTimes[i].repeats);
  }
}

void ApiHandlers::handleSetFeedTimes() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if(server.hasArg("data")) {
    String jsonData = server.arg("data");
    jsonData.trim();

    FeedTime newFeedTimes[FeedingScheduler::MAX_FEED_TIMES];
    int count = parseFeedTimesJson(jsonData, newFeedTimes, FeedingScheduler::MAX_FEED_TIMES);

    if (count == 0) {
      newFeedTimes[0] = {10, 0, 1, false};
      count = 1;
    }

    saveFeedTimes(newFeedTimes, count);
  } else {
    int h1 = server.arg("h1").toInt();
    int m1 = server.arg("m1").toInt();
    int r1 = server.arg("r1").toInt();
    int h2 = server.arg("h2").toInt();
    int m2 = server.arg("m2").toInt();
    int r2 = server.arg("r2").toInt();
    
    scheduler.setLegacyFeedTimes(h1, m1, r1, h2, m2, r2);
    scheduler.saveToPreferences(preferences);
    invalidateCache();
  }
  
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetPowerMode() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  bool updated = false;
  if(server.hasArg("enabled")) {
    powerSaveMode = server.arg("enabled") == "true";
    preferences.putBool("powerSaveMode", powerSaveMode);
    updated = true;
  }
  if (server.hasArg("idleSec")) {
    int v = server.arg("idleSec").toInt();
    setDeepSleepIdleSec(static_cast<uint16_t>(v));
    preferences.putUInt("dsIdleSec", deepSleepIdleSec);
    updated = true;
  }
  if (updated) {
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetDisplayMode() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  bool updated = false;
  if(server.hasArg("enabled")) {
    displayEnabled = server.arg("enabled") == "true";
    preferences.putBool("displayEnabled", displayEnabled);
    Serial.printf("Display %s\n", displayEnabled ? "enabled" : "disabled");
    updated = true;
  }
  if (server.hasArg("sec")) {
    int v = server.arg("sec").toInt();
    setDisplayOffAfterSec(static_cast<uint16_t>(v));
    preferences.putUInt("displayOffSec", displayOffAfterSec);
    Serial.printf("Display off after %u s (power-save ON)\n", displayOffAfterSec);
    updated = true;
  }
  if (updated) {
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetDisplayOff() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if (server.hasArg("sec")) {
    int v = server.arg("sec").toInt();
    setDisplayOffAfterSec(static_cast<uint16_t>(v));
    preferences.putUInt("displayOffSec", displayOffAfterSec);
    invalidateCache();
    Serial.printf("Display off after %u s (power-save ON)\n", displayOffAfterSec);
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetDeepSleep() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if (server.hasArg("idleSec")) {
    int v = server.arg("idleSec").toInt();
    setDeepSleepIdleSec(static_cast<uint16_t>(v));
    preferences.putUInt("dsIdleSec", deepSleepIdleSec);
  }
  invalidateCache();
  Serial.printf("Deep sleep idle: %u s (wake: button + timer before feed)\n", deepSleepIdleSec);
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetTimezone() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if (server.hasArg("offsetMin")) {
    int offsetMin = server.arg("offsetMin").toInt();
    FeedingScheduler::setTimezoneOffsetMinutes(offsetMin);
    preferences.putInt("tzOffsetMin", FeedingScheduler::getTimezoneOffsetMinutes());
    invalidateCache();
    Serial.printf("Timezone offset set to %d minutes\n", FeedingScheduler::getTimezoneOffsetMinutes());
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetFeedInterval() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();
  if (server.hasArg("minutes")) {
    scheduler.setMinFeedIntervalMinutes(server.arg("minutes").toInt());
    preferences.putInt("minFeedGapMin", scheduler.getMinFeedIntervalMinutes());
    invalidateCache();
    Serial.printf("Manual feed interval: %d min\n", scheduler.getMinFeedIntervalMinutes());
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetBatteryCalibration() {
  if (!isTrustedMutationRequest(server)) return;
  noteClientActivity();

  if (!server.hasArg("measuredVoltage")) {
    server.send(400, "text/plain", "measuredVoltage is required");
    return;
  }

  float measuredVoltage = server.arg("measuredVoltage").toFloat();
  if (!isfinite(measuredVoltage) || measuredVoltage <= 0.0f) {
    server.send(400, "text/plain", "invalid measuredVoltage");
    return;
  }

  battery.update();
  float currentVoltage = battery.getVoltage();
  if (!isfinite(currentVoltage) || currentVoltage <= 0.0f) {
    server.send(500, "text/plain", "unable to read battery voltage");
    return;
  }

  float oldCalibration = battery.getCalibrationFactor();
  float newCalibration = oldCalibration * (measuredVoltage / currentVoltage);
  battery.setCalibrationFactor(newCalibration);
  preferences.putFloat("battCal", battery.getCalibrationFactor());
  battery.update();

  invalidateCache();
  server.send(200, "text/plain", "ok");
}

