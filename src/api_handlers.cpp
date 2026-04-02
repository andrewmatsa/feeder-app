#include "api_handlers.h"
#include <Arduino.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "web_pages.h"

ApiHandlers::ApiHandlers(WebServer& server, Preferences& preferences,
                         ServoController& servo, BatteryMonitor& battery,
                         FeedingScheduler& scheduler)
  : server(server), preferences(preferences),
    servo(servo), battery(battery), scheduler(scheduler),
    powerSaveMode(true), displayEnabled(true),
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

String ApiHandlers::buildStatusJson() {
  battery.update();
  
  // ============================================================================
  // ============================================================================
  // ============================================================================
  String json;
  json.reserve(780);
  
  json = "{\"status\":\"ok\",";
  json += "\"currentAngle\":"; json += servo.getCurrentAngle(); json += ",";
  json += "\"speed\":"; json += servo.getSpeed(); json += ",";
  json += "\"feedRepeats\":"; json += feedRepeats; json += ",";
  json += "\"powerSaveMode\":"; json += (powerSaveMode ? "true" : "false"); json += ",";
  json += "\"displayEnabled\":"; json += (displayEnabled ? "true" : "false"); json += ",";
  json += "\"deepSleepIdleSec\":"; json += deepSleepIdleSec; json += ",";
  json += "\"deepSleepWakeButtonOnly\":false,";
  json += "\"batteryVoltage\":"; json += battery.getVoltage(); json += ",";
  json += "\"batteryPercent\":"; json += (int)battery.getPercent(); json += ",";
  
  NextFeedInfo nextFeed = scheduler.computeNextFeed();
  json += "\"nextFeedMinutes\":"; json += nextFeed.minutesUntil; json += ",";
  json += "\"nextFeedHour\":"; json += nextFeed.targetHour; json += ",";
  json += "\"nextFeedMinute\":"; json += nextFeed.targetMinute; json += ",";
  
  json += "\"feedTimes\":[";
  const FeedTime* feedTimes = scheduler.getFeedTimes();
  int feedTimesCount = scheduler.getFeedTimesCount();
  for(int i = 0; i < feedTimesCount; i++) {
    if(i > 0) json += ",";
    json += "{\"h\":"; json += feedTimes[i].hour;
    json += ",\"m\":"; json += feedTimes[i].minute;
    json += ",\"r\":"; json += feedTimes[i].repeats; json += "}";
  }
  json += "],";
  
  int h1, m1, r1, h2, m2, r2;
  scheduler.getLegacyFeedTimes(h1, m1, r1, h2, m2, r2);
  json += "\"feedHour1\":"; json += h1; json += ",";
  json += "\"feedMinute1\":"; json += m1; json += ",";
  json += "\"feedHour2\":"; json += h2; json += ",";
  json += "\"feedMinute2\":"; json += m2; json += ",";
  json += "\"feedRepeats1\":"; json += r1; json += ",";
  json += "\"feedRepeats2\":"; json += r2; json += ",";
  
  time_t now = time(nullptr);
  struct tm localTime;
  char timeBuf[6] = "--:--";
  time_t adjusted = now + FeedingScheduler::KIEV_UTC_OFFSET_SECONDS;
  if (gmtime_r(&adjusted, &localTime) && localTime.tm_year + 1900 >= 2020) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", localTime.tm_hour, localTime.tm_min);
  }
  json += "\"currentTime\":\""; json += timeBuf; json += "\",";
  json += "\"wifiSSID\":\""; json += savedSSID; json += "\",";
  json += "\"isAPMode\":"; json += (isAPMode ? "true" : "false"); json += ",";
  json += "\"sleepReason\":\""; json += sleepReason; json += "\",";
  json += "\"sleepCountdownSeconds\":"; json += sleepCountdownSeconds; json += ",";
  json += "\"displayAwake\":"; json += (displayAwakeNow ? "true" : "false"); json += ",";
  json += "\"manualFeedCooldownSeconds\":"; json += scheduler.getSecondsUntilManualFeedAllowed(); json += ",";
  if(!isAPMode && WiFi.status() == WL_CONNECTED) {
    json += "\"wifiIP\":\""; json += WiFi.localIP().toString(); json += "\",";
  } else {
    json += "\"wifiIP\":\"\",";
  }
  
  // ============================================================================
  // ============================================================================
  unsigned long freeHeap = ESP.getFreeHeap();
  unsigned long maxAllocHeap = ESP.getMaxAllocHeap();
  unsigned long minFreeHeap = ESP.getMinFreeHeap();
  unsigned long totalHeap = ESP.getHeapSize();
  
  json += "\"memoryFreeHeap\":"; json += freeHeap; json += ",";
  json += "\"memoryMaxAllocHeap\":"; json += maxAllocHeap; json += ",";
  json += "\"memoryMinFreeHeap\":"; json += minFreeHeap; json += ",";
  json += "\"memoryTotalHeap\":"; json += totalHeap; json += ",";
  json += "\"memoryUsedHeap\":"; json += (totalHeap - freeHeap); json += ",";
  
  unsigned long cacheAge = (cachedStatusJson.length() > 0 && cachedStatusTime > 0) 
    ? (millis() - cachedStatusTime) : 0;
  json += "\"cacheSize\":"; json += cachedStatusJson.length(); json += ",";
  json += "\"cacheAge\":"; json += cacheAge; json += ",";
  json += "\"cacheValid\":"; json += (cachedStatusJson.length() > 0 ? "true" : "false"); json += ",";
  
  json += "\"cpuFrequency\":"; json += getCpuFrequencyMhz();
  json += "}";
  
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

void ApiHandlers::handleSetFeedTimes() {
  noteClientActivity();
  if(server.hasArg("data")) {
    String jsonData = server.arg("data");
    jsonData.trim();
    
    FeedTime newFeedTimes[FeedingScheduler::MAX_FEED_TIMES];
    int count = 0;
    
    int depth = 0;
    int objStart = -1;
    const int len = jsonData.length();
    for(int idx = 0; idx < len && count < FeedingScheduler::MAX_FEED_TIMES; idx++) {
      char c = jsonData.charAt(idx);
      if(c == '{') {
        if(depth == 0) {
          objStart = idx;
        }
        depth++;
      } else if(c == '}') {
        depth--;
        if(depth == 0 && objStart != -1) {
          String obj = jsonData.substring(objStart + 1, idx);
          int h = extractIntField(obj, 'h', 10);
          int m = extractIntField(obj, 'm', 0);
          int r = extractIntField(obj, 'r', 1);
          
          h = constrain(h, 0, 23);
          m = constrain(m, 0, 59);
          r = max(1, r);
          
          newFeedTimes[count] = {h, m, r, false};
          count++;
          objStart = -1;
        }
      }
    }
    
    if(count == 0) {
      newFeedTimes[0] = {10, 0, 1, false};
      count = 1;
    }
    
    scheduler.setFeedTimes(newFeedTimes, count);
    scheduler.saveToPreferences(preferences);
    invalidateCache();
    Serial.printf("Feed times saved: count=%d\n", count);
    for(int i = 0; i < count; i++) {
      Serial.printf("  [%d] %02d:%02d (repeats=%d)\n", i, newFeedTimes[i].hour, newFeedTimes[i].minute, newFeedTimes[i].repeats);
    }
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

bool ApiHandlers::isDigitChar(char c) {
  return c >= '0' && c <= '9';
}

int ApiHandlers::extractIntField(const String& obj, char fieldKey, int fallback) {
  String pattern = "\"";
  pattern += fieldKey;
  pattern += "\":";
  int pos = obj.indexOf(pattern);
  if (pos == -1) {
    String shortPattern = "";
    shortPattern += fieldKey;
    shortPattern += ":";
    pos = obj.indexOf(shortPattern);
    if (pos == -1) return fallback;
  }
  
  int colon = obj.indexOf(':', pos);
  if (colon == -1) return fallback;
  
  int valueStart = colon + 1;
  while (valueStart < obj.length()) {
    char c = obj.charAt(valueStart);
    if (c == ' ' || c == '\t' || c == '"' || c == '\'') {
      valueStart++;
      continue;
    }
    break;
  }
  if (valueStart >= obj.length()) return fallback;
  
  bool negative = false;
  if (obj.charAt(valueStart) == '-') {
    negative = true;
    valueStart++;
  }
  
  int valueEnd = valueStart;
  while (valueEnd < obj.length() && isDigitChar(obj.charAt(valueEnd))) {
    valueEnd++;
  }
  
  if (valueEnd == valueStart) return fallback;
  
  int value = obj.substring(negative ? valueStart - 1 : valueStart, valueEnd).toInt();
  return value;
}

