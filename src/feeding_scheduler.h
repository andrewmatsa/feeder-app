#ifndef FEEDING_SCHEDULER_H
#define FEEDING_SCHEDULER_H

#include <Preferences.h>
#include "time.h"

struct FeedTime {
  int hour;
  int minute;
  int repeats;
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
  static constexpr long KIEV_UTC_OFFSET_SECONDS = 2 * 3600; // UTC+2
  
  FeedingScheduler();
  void begin(Preferences& preferences);
  void loadFromPreferences(Preferences& preferences);
  void saveToPreferences(Preferences& preferences);
  
  void setFeedTimes(const FeedTime* times, int count);
  int getFeedTimesCount() const { return feedTimesCount; }
  const FeedTime* getFeedTimes() const { return feedTimes; }
  
  bool checkAndFeed(void (*feedCallback)(int repeats));
  void loop(void (*feedCallback)(int repeats) = nullptr);
  NextFeedInfo computeNextFeed();
  
  void recordManualFeed();
  
  void setLegacyFeedTimes(int h1, int m1, int r1, int h2, int m2, int r2);
  void getLegacyFeedTimes(int& h1, int& m1, int& r1, int& h2, int& m2, int& r2) const;
  
private:
  FeedTime feedTimes[MAX_FEED_TIMES];
  int feedTimesCount;
  
  int feedHour1, feedMinute1, feedRepeats1;
  int feedHour2, feedMinute2, feedRepeats2;
  bool feed1Done, feed2Done;
  
  int lastCheckedHour;
  int lastCheckedMinute;
  
  int lastFedHour;
  int lastFedMinute;
  static const int MIN_FEED_INTERVAL_MINUTES = 5;
  
  bool isTimeForFeeding(int hour, int minute, int feedHour, int feedMinute, bool& doneFlag);
  void resetFlagsForPreviousTime();
  bool wasRecentlyFed(int currentHour, int currentMinute);
};

#endif

