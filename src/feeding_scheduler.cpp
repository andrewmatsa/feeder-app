#include "feeding_scheduler.h"
#include <Arduino.h>
#include <Preferences.h>

int FeedingScheduler::timezoneOffsetMinutes = FeedingScheduler::DEFAULT_TIMEZONE_OFFSET_MINUTES;

FeedingScheduler::FeedingScheduler()
  : feedTimesCount(0),
    lastCheckedHour(-1), lastCheckedMinute(-1),
    lastFedHour(-1), lastFedMinute(-1),
    lastFedEpochMinute(-1),
    lastSchedEpMinute(-1),
    minFeedIntervalMinutes(DEFAULT_MIN_FEED_INTERVAL_MINUTES),
    preferencesRef(nullptr) {
  for(int i = 0; i < MAX_FEED_TIMES; i++) {
    feedTimes[i] = {0, 0, 1, -1, false};
  }
}

void FeedingScheduler::begin(Preferences& preferences) {
  preferencesRef = &preferences;
  setTimezoneOffsetMinutes(preferences.getInt("tzOffsetMin", DEFAULT_TIMEZONE_OFFSET_MINUTES));
  loadFromPreferences(preferences);
  lastFedEpochMinute = preferences.getLong64("lastFeedEpMin", -1);
  lastSchedEpMinute = preferences.getLong64("lastSchedEpMin", -1);
}

void FeedingScheduler::setTimezoneOffsetMinutes(int minutes) {
  if (minutes < -720) {
    minutes = -720;
  }
  if (minutes > 840) {
    minutes = 840;
  }
  timezoneOffsetMinutes = minutes;
}

int FeedingScheduler::getTimezoneOffsetMinutes() {
  return timezoneOffsetMinutes;
}

long FeedingScheduler::getTimezoneOffsetSeconds() {
  return static_cast<long>(timezoneOffsetMinutes) * 60L;
}

bool FeedingScheduler::toLocalTime(time_t now, struct tm& localTime, long long* epochMinute) {
  time_t adjusted = now + getTimezoneOffsetSeconds();
  if (!gmtime_r(&adjusted, &localTime) || localTime.tm_year + 1900 < 2020) {
    return false;
  }

  if (epochMinute) {
    *epochMinute = static_cast<long long>(now / 60);
  }
  return true;
}

void FeedingScheduler::loadFromPreferences(Preferences& preferences) {
  setMinFeedIntervalMinutes(preferences.getInt("minFeedGapMin", DEFAULT_MIN_FEED_INTERVAL_MINUTES));
  feedTimesCount = preferences.getInt("feedTimesCount", 0);
  Serial.printf("Loading feed times from preferences: count=%d\n", feedTimesCount);
  
  if(feedTimesCount <= 0 || feedTimesCount > MAX_FEED_TIMES) {
    feedTimesCount = 0;
    
    int storedH1 = preferences.getInt("feedHour1", -1);
    int storedM1 = preferences.getInt("feedMinute1", -1);
    int storedR1 = preferences.getInt("feedRepeats1", -1);
    
    if(storedH1 < 0) storedH1 = 10;
    if(storedM1 < 0) storedM1 = 0;
    if(storedR1 < 0) storedR1 = 1;
    
    feedTimes[feedTimesCount++] = {storedH1, storedM1, storedR1, -1, false};
    
    if(preferences.isKey("feedHour2") && preferences.isKey("feedMinute2")) {
      int storedH2 = preferences.getInt("feedHour2", storedH1);
      int storedM2 = preferences.getInt("feedMinute2", storedM1);
      int storedR2 = preferences.getInt("feedRepeats2", storedR1);
      feedTimes[feedTimesCount++] = {storedH2, storedM2, storedR2, -1, false};
    }
  } else {
    char key[20];
    for(int i = 0; i < feedTimesCount; i++) {
      sprintf(key, "feedH%d", i);
      feedTimes[i].hour = preferences.getInt(key, 10);
      sprintf(key, "feedM%d", i);
      feedTimes[i].minute = preferences.getInt(key, 0);
      sprintf(key, "feedR%d", i);
      feedTimes[i].repeats = preferences.getInt(key, 1);
      sprintf(key, "feedD%d", i);
      feedTimes[i].day = preferences.getInt(key, -1);
      feedTimes[i].done = false;
      Serial.printf("  Loaded [%d]: %02d:%02d (repeats=%d, day=%d)\n", i, feedTimes[i].hour, feedTimes[i].minute, feedTimes[i].repeats, feedTimes[i].day);
    }
  }
  
}

void FeedingScheduler::saveToPreferences(Preferences& preferences) {
  preferences.putInt("minFeedGapMin", minFeedIntervalMinutes);
  preferences.putInt("feedTimesCount", feedTimesCount);
  Serial.printf("Saving %d feed times to preferences\n", feedTimesCount);
  char key[20];
  for(int i = 0; i < feedTimesCount; i++) {
    sprintf(key, "feedH%d", i);
    preferences.putInt(key, feedTimes[i].hour);
    sprintf(key, "feedM%d", i);
    preferences.putInt(key, feedTimes[i].minute);
    sprintf(key, "feedR%d", i);
    preferences.putInt(key, feedTimes[i].repeats);
    sprintf(key, "feedD%d", i);
    preferences.putInt(key, feedTimes[i].day);
    Serial.printf("  Saved [%d]: %02d:%02d (repeats=%d, day=%d)\n", i, feedTimes[i].hour, feedTimes[i].minute, feedTimes[i].repeats, feedTimes[i].day);
  }
  
}

void FeedingScheduler::setFeedTimes(const FeedTime* times, int count) {
  feedTimesCount = min(count, MAX_FEED_TIMES);
  for(int i = 0; i < feedTimesCount; i++) {
    feedTimes[i] = times[i];
    feedTimes[i].done = false;
  }
  
}

void FeedingScheduler::setLegacyFeedTimes(int h1, int m1, int r1, int h2, int m2, int r2) {
  feedTimesCount = 2;
  feedTimes[0] = {h1, m1, r1, -1, false};
  feedTimes[1] = {h2, m2, r2, -1, false};
}

void FeedingScheduler::getLegacyFeedTimes(int& h1, int& m1, int& r1, int& h2, int& m2, int& r2) const {
  h1 = feedTimesCount > 0 ? feedTimes[0].hour : 10;
  m1 = feedTimesCount > 0 ? feedTimes[0].minute : 0;
  r1 = feedTimesCount > 0 ? feedTimes[0].repeats : 1;
  h2 = feedTimesCount > 1 ? feedTimes[1].hour : 0;
  m2 = feedTimesCount > 1 ? feedTimes[1].minute : 0;
  r2 = feedTimesCount > 1 ? feedTimes[1].repeats : 1;
}

NextFeedInfo FeedingScheduler::computeNextFeed() {
  NextFeedInfo info;
  time_t now = time(nullptr);
  struct tm localTime;
  if (!toLocalTime(now, localTime)) {
    return info;
  }
  
  const int nowTotal = localTime.tm_hour * 60 + localTime.tm_min;
  const int nowDay = localTime.tm_wday;
  // Must exceed the largest possible wraparound diff: weekday-specific slots
  // (day >= 0) can wrap a full week (7*24*60), not just a day.
  int bestDiff = (7 * 24 * 60) + 1;
  bool found = false;
  
  auto considerSlot = [&](int hour, int minute, int day) {
    if (hour < 0 || minute < 0) return;
    hour = constrain(hour, 0, 23);
    minute = constrain(minute, 0, 59);
    day = constrain(day, -1, 6);
    int slotTotal = hour * 60 + minute;
    int dayOffset = 0;
    if (day >= 0) {
      dayOffset = day - nowDay;
      if (dayOffset < 0) dayOffset += 7;
    }
    int diff = dayOffset * 24 * 60 + (slotTotal - nowTotal);
    if (diff <= 0) {
      diff += (day >= 0) ? (7 * 24 * 60) : (24 * 60);
    }
    if (diff < bestDiff) {
      bestDiff = diff;
      info.minutesUntil = diff;
      info.targetHour = hour;
      info.targetMinute = minute;
      found = true;
    }
  };
  
  for (int i = 0; i < feedTimesCount; ++i) {
    considerSlot(feedTimes[i].hour, feedTimes[i].minute, feedTimes[i].day);
  }

  return info;
}

bool FeedingScheduler::wasRecentlyFed(int currentHour, int currentMinute) {
  struct tm localTime;
  long long currentEpochMinute = -1;
  if (getCurrentLocalTime(localTime, &currentEpochMinute) &&
      lastFedEpochMinute >= 0 &&
      currentEpochMinute >= lastFedEpochMinute) {
    long long diff = currentEpochMinute - lastFedEpochMinute;
    if (diff < minFeedIntervalMinutes) {
      return true;
    }
    return false;
  }

  if (lastFedHour < 0 || lastFedMinute < 0) {
    return false;
  }
  
  int currentTotal = currentHour * 60 + currentMinute;
  int lastFedTotal = lastFedHour * 60 + lastFedMinute;
  int diff = currentTotal - lastFedTotal;
  
  if (diff < 0) {
    diff += 24 * 60;
  }
  
  if (diff < minFeedIntervalMinutes) {
    return true;
  }
  
  return false;
}

bool FeedingScheduler::getCurrentLocalTime(struct tm& localTime, long long* epochMinute) {
  time_t now = time(nullptr);
  return toLocalTime(now, localTime, epochMinute);
}

void FeedingScheduler::updateLastFeedState(const struct tm& localTime, long long epochMinute) {
  lastFedHour = localTime.tm_hour;
  lastFedMinute = localTime.tm_min;
  lastFedEpochMinute = epochMinute;
  persistLastFeedState();
}

void FeedingScheduler::persistLastFeedState() {
  if (!preferencesRef) {
    return;
  }
  preferencesRef->putLong64("lastFeedEpMin", lastFedEpochMinute);
}

bool FeedingScheduler::wasScheduleAlreadyExecuted(long long epochMinute) const {
  return lastSchedEpMinute >= 0 && lastSchedEpMinute == epochMinute;
}

void FeedingScheduler::persistLastScheduledFeedState() {
  if (!preferencesRef) {
    return;
  }
  preferencesRef->putLong64("lastSchedEpMin", lastSchedEpMinute);
}

bool FeedingScheduler::checkAndFeed(void (*feedCallback)(int repeats)) {
  time_t now = time(nullptr);
  struct tm localTime;
  long long currentEpochMinute = -1;
  if (!toLocalTime(now, localTime, &currentEpochMinute)) {
    return false;
  }
  
  int curHour = localTime.tm_hour;
  int curMinute = localTime.tm_min;
  int curDay = localTime.tm_wday;
  
  if (wasScheduleAlreadyExecuted(currentEpochMinute)) {
    for (int i = 0; i < feedTimesCount; i++) {
      if (curHour == feedTimes[i].hour && curMinute == feedTimes[i].minute) {
        feedTimes[i].done = true;
      }
    }
    return false;
  }
  
  if (lastCheckedHour != -1 && curHour != lastCheckedHour) {
    for(int i = 0; i < feedTimesCount; i++) {
      if (feedTimes[i].hour == lastCheckedHour) {
        feedTimes[i].done = false;
      }
    }
  }

  if (lastCheckedHour != -1 && lastCheckedMinute != -1) {
    int timeDiff = (curHour * 60 + curMinute) - (lastCheckedHour * 60 + lastCheckedMinute);
    if (timeDiff < -10 && timeDiff > -1430) {
      Serial.println("Warning: Time jumped backwards, resetting feed flags");
      for(int i = 0; i < feedTimesCount; i++) {
        feedTimes[i].done = false;
      }
    }
  }
  
  bool fed = false;
  int repeatsToRun = 0;
  
  for(int i = 0; i < feedTimesCount; i++) {
    bool dayMatches = (feedTimes[i].day < 0) || (feedTimes[i].day == curDay);
    if (dayMatches && curHour == feedTimes[i].hour && curMinute == feedTimes[i].minute && !feedTimes[i].done) {
      repeatsToRun += max(1, feedTimes[i].repeats);
      feedTimes[i].done = true;
      ets_printf("[SCHED] Slot %d triggered at %02d:%02d, repeats: %d\n", i+1, curHour, curMinute, feedTimes[i].repeats);
    }
  }

  if (repeatsToRun > 0) {
    ets_printf("[SCHED] Running auto feed at %02d:%02d, total repeats: %d\n", curHour, curMinute, repeatsToRun);
    if (feedCallback) {
      feedCallback(repeatsToRun);
    }
    lastSchedEpMinute = currentEpochMinute;
    persistLastScheduledFeedState();
    updateLastFeedState(localTime, currentEpochMinute);
    fed = true;
  }
  
  lastCheckedHour = curHour;
  lastCheckedMinute = curMinute;
  
  return fed;
}

void FeedingScheduler::loop(void (*feedCallback)(int repeats)) {
  checkAndFeed(feedCallback);
}

bool FeedingScheduler::canFeedNow() {
  struct tm localTime;
  long long epochMinute = -1;
  if (!getCurrentLocalTime(localTime, &epochMinute)) {
    return true;
  }

  return !wasRecentlyFed(localTime.tm_hour, localTime.tm_min);
}

int FeedingScheduler::getSecondsUntilManualFeedAllowed() const {
  if (lastFedEpochMinute < 0) {
    return 0;
  }
  long long nowEpochSeconds = static_cast<long long>(time(nullptr));
  long long curEm = nowEpochSeconds / 60;
  long long diffMin = curEm - lastFedEpochMinute;
  if (diffMin >= minFeedIntervalMinutes) {
    return 0;
  }
  long long endEpochSeconds = (lastFedEpochMinute + minFeedIntervalMinutes) * 60LL;
  long long remain = endEpochSeconds - nowEpochSeconds;
  if (remain <= 0) {
    return 0;
  }
  if (remain > 86400) {
    return 86400;
  }
  return static_cast<int>(remain);
}

void FeedingScheduler::recordManualFeed() {
  struct tm localTime;
  long long epochMinute = -1;
  if (getCurrentLocalTime(localTime, &epochMinute)) {
    updateLastFeedState(localTime, epochMinute);
    Serial.printf("Manual feed recorded at %02d:%02d\n", lastFedHour, lastFedMinute);
  }
}

void FeedingScheduler::setMinFeedIntervalMinutes(int minutes) {
  if (minutes < MIN_ALLOWED_FEED_INTERVAL_MINUTES) {
    minutes = MIN_ALLOWED_FEED_INTERVAL_MINUTES;
  }
  if (minutes > MAX_ALLOWED_FEED_INTERVAL_MINUTES) {
    minutes = MAX_ALLOWED_FEED_INTERVAL_MINUTES;
  }
  minFeedIntervalMinutes = minutes;
}

