#include "feeding_scheduler.h"
#include <Arduino.h>
#include <Preferences.h>

int FeedingScheduler::timezoneOffsetMinutes = FeedingScheduler::DEFAULT_TIMEZONE_OFFSET_MINUTES;

FeedingScheduler::FeedingScheduler()
  : feedTimesCount(0),
    feedHour1(10), feedMinute1(0), feedRepeats1(1),
    feedHour2(20), feedMinute2(0), feedRepeats2(1),
    feed1Done(false), feed2Done(false),
    lastCheckedHour(-1), lastCheckedMinute(-1),
    lastFedHour(-1), lastFedMinute(-1),
    lastFedEpochMinute(-1),
    lastScheduledFeedEpochMinute(-1),
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
  lastFedEpochMinute = preferences.getLong64("lastFeedEpochMin", -1);
  lastScheduledFeedEpochMinute = preferences.getLong64("lastScheduledFeedEpochMin", -1);
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
  
  if(feedTimesCount > 0) {
    feedHour1 = feedTimes[0].hour;
    feedMinute1 = feedTimes[0].minute;
    feedRepeats1 = feedTimes[0].repeats;
  }
  if(feedTimesCount > 1) {
    feedHour2 = feedTimes[1].hour;
    feedMinute2 = feedTimes[1].minute;
    feedRepeats2 = feedTimes[1].repeats;
  } else {
    feedHour2 = 0;
    feedMinute2 = 0;
    feedRepeats2 = 1;
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
  
  preferences.putInt("feedHour1", feedHour1);
  preferences.putInt("feedMinute1", feedMinute1);
  preferences.putInt("feedHour2", feedHour2);
  preferences.putInt("feedMinute2", feedMinute2);
  preferences.putInt("feedRepeats1", feedRepeats1);
  preferences.putInt("feedRepeats2", feedRepeats2);
}

void FeedingScheduler::setFeedTimes(const FeedTime* times, int count) {
  feedTimesCount = min(count, MAX_FEED_TIMES);
  for(int i = 0; i < feedTimesCount; i++) {
    feedTimes[i] = times[i];
    feedTimes[i].done = false;
  }
  
  if(feedTimesCount > 0) {
    feedHour1 = feedTimes[0].hour;
    feedMinute1 = feedTimes[0].minute;
    feedRepeats1 = feedTimes[0].repeats;
  }
  if(feedTimesCount > 1) {
    feedHour2 = feedTimes[1].hour;
    feedMinute2 = feedTimes[1].minute;
    feedRepeats2 = feedTimes[1].repeats;
  }
}

void FeedingScheduler::setLegacyFeedTimes(int h1, int m1, int r1, int h2, int m2, int r2) {
  feedHour1 = h1; feedMinute1 = m1; feedRepeats1 = r1;
  feedHour2 = h2; feedMinute2 = m2; feedRepeats2 = r2;
  
  feedTimesCount = 2;
  feedTimes[0] = {feedHour1, feedMinute1, feedRepeats1, -1, false};
  feedTimes[1] = {feedHour2, feedMinute2, feedRepeats2, -1, false};
}

void FeedingScheduler::getLegacyFeedTimes(int& h1, int& m1, int& r1, int& h2, int& m2, int& r2) const {
  h1 = feedHour1; m1 = feedMinute1; r1 = feedRepeats1;
  h2 = feedHour2; m2 = feedMinute2; r2 = feedRepeats2;
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
  int bestDiff = (24 * 60) + 1;
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
  
  if (!found) {
    considerSlot(feedHour1, feedMinute1, -1);
    considerSlot(feedHour2, feedMinute2, -1);
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
      Serial.printf("Feed blocked: only %lld minutes since last feed at %02d:%02d\n",
                    diff, lastFedHour, lastFedMinute);
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
    Serial.printf("Feed blocked: only %d minutes since last feed at %02d:%02d\n", diff, lastFedHour, lastFedMinute);
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
  preferencesRef->putLong64("lastFeedEpochMin", lastFedEpochMinute);
}

bool FeedingScheduler::wasScheduleAlreadyExecuted(long long epochMinute) const {
  return lastScheduledFeedEpochMinute >= 0 && lastScheduledFeedEpochMinute == epochMinute;
}

void FeedingScheduler::persistLastScheduledFeedState() {
  if (!preferencesRef) {
    return;
  }
  preferencesRef->putLong64("lastScheduledFeedEpochMin", lastScheduledFeedEpochMinute);
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
    if (feedTimesCount == 0) {
      if (curHour == feedHour1 && curMinute == feedMinute1) {
        feed1Done = true;
      }
      if (curHour == feedHour2 && curMinute == feedMinute2) {
        feed2Done = true;
      }
    }
    Serial.printf("Scheduled feed for %02d:%02d already executed, skipping duplicate\n", curHour, curMinute);
    return false;
  }
  
  if (lastCheckedHour != -1 && curHour != lastCheckedHour) {
    for(int i = 0; i < feedTimesCount; i++) {
      if (feedTimes[i].hour == lastCheckedHour) {
        feedTimes[i].done = false;
      }
    }
    if (feedTimesCount == 0) {
      if (feedHour1 == lastCheckedHour) {
        feed1Done = false;
      }
      if (feedHour2 == lastCheckedHour) {
        feed2Done = false;
      }
    }
  }
  
  if (lastCheckedHour != -1 && lastCheckedMinute != -1) {
    int timeDiff = (curHour * 60 + curMinute) - (lastCheckedHour * 60 + lastCheckedMinute);
    if (timeDiff < -10 && timeDiff > -1430) {
      Serial.println("Warning: Time jumped backwards significantly, resetting all feed flags");
      for(int i = 0; i < feedTimesCount; i++) {
        feedTimes[i].done = false;
      }
      feed1Done = false;
      feed2Done = false;
    }
  }
  
  bool fed = false;
  int repeatsToRun = 0;
  
  for(int i = 0; i < feedTimesCount; i++) {
    bool dayMatches = (feedTimes[i].day < 0) || (feedTimes[i].day == curDay);
    if (dayMatches && curHour == feedTimes[i].hour && curMinute == feedTimes[i].minute && !feedTimes[i].done) {
      repeatsToRun += max(1, feedTimes[i].repeats);
      feedTimes[i].done = true;
      Serial.printf("Auto feeding queued (slot %d) day=%d %02d:%02d, repeats: %d\n", i+1, feedTimes[i].day, curHour, curMinute, feedTimes[i].repeats);
    }
  }

  if (repeatsToRun > 0) {
    Serial.printf("Auto feeding total %02d:%02d, combined repeats: %d\n", curHour, curMinute, repeatsToRun);
    if (feedCallback) {
      feedCallback(repeatsToRun);
    }
    lastScheduledFeedEpochMinute = currentEpochMinute;
    persistLastScheduledFeedState();
    updateLastFeedState(localTime, currentEpochMinute);
    fed = true;
  }
  
  if (feedTimesCount == 0) {
    int legacyRepeatsToRun = 0;
    if (curHour == feedHour1 && curMinute == feedMinute1 && !feed1Done) {
      legacyRepeatsToRun += max(1, feedRepeats1);
      feed1Done = true;
      Serial.printf("Auto feeding queued (slot 1 legacy) %02d:%02d, repeats: %d\n", curHour, curMinute, feedRepeats1);
    }
    if (curHour == feedHour2 && curMinute == feedMinute2 && !feed2Done) {
      legacyRepeatsToRun += max(1, feedRepeats2);
      feed2Done = true;
      Serial.printf("Auto feeding queued (slot 2 legacy) %02d:%02d, repeats: %d\n", curHour, curMinute, feedRepeats2);
    }

    if (legacyRepeatsToRun > 0) {
      Serial.printf("Auto feeding total legacy %02d:%02d, combined repeats: %d\n", curHour, curMinute, legacyRepeatsToRun);
      if (feedCallback) {
        feedCallback(legacyRepeatsToRun);
      }
      lastScheduledFeedEpochMinute = currentEpochMinute;
      persistLastScheduledFeedState();
      updateLastFeedState(localTime, currentEpochMinute);
      fed = true;
    }
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

