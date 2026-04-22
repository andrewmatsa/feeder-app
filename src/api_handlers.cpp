#include "api_handlers.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "web_pages.h"

ApiHandlers::ApiHandlers(WebServer& server, Preferences& preferences,
                         ServoController& servo, BatteryMonitor& battery,
                         FeedingScheduler& scheduler)
  : server(server), preferences(preferences),
    servo(servo), battery(battery), scheduler(scheduler),
    powerSaveMode(true), displayEnabled(true),
    displayOffAfterSec(20),
    deepSleepIdleSec(60),
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
  server.on("/", [this]() { handleRoot(); });
  server.on("/info", [this]() { handleInfo(); });
  server.on("/api/status", [this]() { handleStatus(); });
  server.on("/api/setAngle", [this]() { handleSetAngle(); });
  server.on("/api/feedNow", [this]() { handleFeedNow(); });
  server.on("/api/setSpeed", [this]() { handleSetSpeed(); });
  server.on("/api/setRepeats", [this]() { handleSetRepeats(); });
  server.on("/api/setFeedTimes", [this]() { handleSetFeedTimes(); });
  server.on("/api/setPowerMode", [this]() { handleSetPowerMode(); });
  server.on("/api/setDisplayMode", [this]() { handleSetDisplayMode(); });
  server.on("/api/setDisplayOff", [this]() { handleSetDisplayOff(); });
  server.on("/api/setDeepSleep", [this]() { handleSetDeepSleep(); });
}

void ApiHandlers::handleRoot() {
  noteClientActivity();
  if (isAPMode || WiFi.status() != WL_CONNECTED) {
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(200, "text/html", pageIndex);
}

void ApiHandlers::handleInfo() {
  noteClientActivity();
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
  time_t adjusted = now + FeedingScheduler::KIEV_UTC_OFFSET_SECONDS;
  if (gmtime_r(&adjusted, &localTime) && localTime.tm_year + 1900 >= 2020) {
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
  noteClientActivity();
  if(server.hasArg("angle") && !servo.isMoving()) {
    servo.setAngle(server.arg("angle").toInt(), false);
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleFeedNow() {
  noteClientActivity();
  if (!scheduler.canFeedNow()) {
    server.send(429, "text/plain", "feeding blocked: recently fed");
    return;
  }

  server.send(200, "text/plain", "feeding");
  delay(10);

  servo.feedSequence(feedRepeats);
  scheduler.recordManualFeed();
  invalidateCache();
}

void ApiHandlers::handleSetSpeed() {
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
    int h = constrain(item["h"] | item["hour"] | 10, 0, 23);
    int m = constrain(item["m"] | item["minute"] | 0, 0, 59);
    int r = max(1, static_cast<int>(item["r"] | item["repeats"] | 1));
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
  noteClientActivity();
  if(server.hasArg("enabled")) {
    powerSaveMode = server.arg("enabled") == "true";
    preferences.putBool("powerSaveMode", powerSaveMode);
    invalidateCache();
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetDisplayMode() {
  noteClientActivity();
  if(server.hasArg("enabled")) {
    displayEnabled = server.arg("enabled") == "true";
    preferences.putBool("displayEnabled", displayEnabled);
    invalidateCache();
    Serial.printf("Display %s\n", displayEnabled ? "enabled" : "disabled");
  }
  server.send(200, "text/plain", "ok");
}

void ApiHandlers::handleSetDisplayOff() {
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

