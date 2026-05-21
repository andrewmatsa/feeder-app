#ifndef FEEDING_SCHEDULER_H
#define FEEDING_SCHEDULER_H

#include <Preferences.h>
#include "time.h"

struct FeedTime {
  int hour;
  int minute;
  int repeats;
  int day;   // -1 everyday, 0 Sunday ... 6 Saturday
  bool done;
};

struct NextFeedInfo {
  int minutesUntil = -1;
  int targetHour = -1;
  int targetMinute = -1;
};

class FeedingScheduler {
public:
  static const int MAX_FEED_TIMES = 20;
  static const int DEFAULT_TIMEZONE_OFFSET_MINUTES = 120; // UTC+2
  
  FeedingScheduler();
  void begin(Preferences& preferences);
  void loadFromPreferences(Preferences& preferences);
  void saveToPreferences(Preferences& preferences);
  static void setTimezoneOffsetMinutes(int minutes);
  static int getTimezoneOffsetMinutes();
  static long getTimezoneOffsetSeconds();
  static bool toLocalTime(time_t now, struct tm& localTime, long long* epochMinute = nullptr);
  
  void setFeedTimes(const FeedTime* times, int count);
  int getFeedTimesCount() const { return feedTimesCount; }
  const FeedTime* getFeedTimes() const { return feedTimes; }
  
  bool checkAndFeed(void (*feedCallback)(int repeats));
  void loop(void (*feedCallback)(int repeats) = nullptr);
  NextFeedInfo computeNextFeed();
  
  bool canFeedNow();
  /** Seconds until manual feed is allowed; 0 if allowed now or time unknown. */
  int getSecondsUntilManualFeedAllowed() const;
  void recordManualFeed();
  void setMinFeedIntervalMinutes(int minutes);
  int getMinFeedIntervalMinutes() const { return minFeedIntervalMinutes; }
  
  void setLegacyFeedTimes(int h1, int m1, int r1, int h2, int m2, int r2);
  void getLegacyFeedTimes(int& h1, int& m1, int& r1, int& h2, int& m2, int& r2) const;
  
private:
  FeedTime feedTimes[MAX_FEED_TIMES];
  int feedTimesCount;

  int lastCheckedHour;
  int lastCheckedMinute;
  
  int lastFedHour;
  int lastFedMinute;
  long long lastFedEpochMinute;
  long long lastSchedEpMinute;
  int minFeedIntervalMinutes;
  Preferences* preferencesRef;
  static int timezoneOffsetMinutes;
  static const int DEFAULT_MIN_FEED_INTERVAL_MINUTES = 5;
  static const int MIN_ALLOWED_FEED_INTERVAL_MINUTES = 1;
  static const int MAX_ALLOWED_FEED_INTERVAL_MINUTES = 1440;
  
  bool wasRecentlyFed(int currentHour, int currentMinute);
  bool getCurrentLocalTime(struct tm& localTime, long long* epochMinute = nullptr);
  void updateLastFeedState(const struct tm& localTime, long long epochMinute);
  void persistLastFeedState();
  bool wasScheduleAlreadyExecuted(long long epochMinute) const;
  void persistLastScheduledFeedState();
};

#endif

