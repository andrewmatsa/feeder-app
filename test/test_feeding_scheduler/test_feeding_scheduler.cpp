#include <unity.h>

// Pull in the REAL implementation (not a hand-copied mirror) so a regression
// in the actual scheduling logic fails this test, using the mock
// Arduino.h/Preferences.h on test/native_mocks (see platformio.ini env:native
// -Itest/native_mocks) to satisfy its framework dependencies.
#include "../../src/feeding_scheduler.cpp"

// FeedingScheduler::MAX_FEED_TIMES is declared `static const int` in the
// header without an out-of-class definition; taking its address here (via
// Unity's assert macros) needs storage that only this test binary requires.
const int FeedingScheduler::MAX_FEED_TIMES;

#include <cmath>

void setUp() {}
void tearDown() {}

// ── helpers ───────────────────────────────────────────────────────────────────

static void nowLocal(struct tm& out) {
  time_t now = time(nullptr);
  TEST_ASSERT_TRUE(FeedingScheduler::toLocalTime(now, out));
}

static FeedTime slot(int hour, int minute, int repeats = 1, int day = -1) {
  return FeedTime{hour, minute, repeats, day, false};
}

// ── computeNextFeed: deterministic (no real-clock dependency) ────────────────

void test_compute_next_feed_returns_default_when_no_slots() {
  FeedingScheduler scheduler;
  NextFeedInfo info = scheduler.computeNextFeed();
  TEST_ASSERT_EQUAL(-1, info.minutesUntil);
  TEST_ASSERT_EQUAL(-1, info.targetHour);
  TEST_ASSERT_EQUAL(-1, info.targetMinute);
}

// ── computeNextFeed: relative-to-now (real wall clock, tolerant assertions) ──
// FeedingScheduler calls time(nullptr) internally rather than taking a clock
// dependency, so these tests anchor slots relative to the actual current
// time instead of a fixed timestamp.

void test_compute_next_feed_picks_slot_later_today() {
  struct tm local;
  nowLocal(local);
  int nowMinutes = local.tm_hour * 60 + local.tm_min;
  int targetMinutes = (nowMinutes + 15) % (24 * 60);

  FeedTime times[] = {slot(targetMinutes / 60, targetMinutes % 60)};
  FeedingScheduler scheduler;
  scheduler.setFeedTimes(times, 1);

  NextFeedInfo info = scheduler.computeNextFeed();
  TEST_ASSERT_INT_WITHIN(1, 15, info.minutesUntil);
  TEST_ASSERT_EQUAL(targetMinutes / 60, info.targetHour);
  TEST_ASSERT_EQUAL(targetMinutes % 60, info.targetMinute);
}

void test_compute_next_feed_wraps_to_tomorrow_when_slot_already_passed() {
  struct tm local;
  nowLocal(local);
  int nowMinutes = local.tm_hour * 60 + local.tm_min;
  // 10 minutes ago (mod a day) — an "everyday" slot already missed today.
  int targetMinutes = ((nowMinutes - 10) % (24 * 60) + 24 * 60) % (24 * 60);

  FeedTime times[] = {slot(targetMinutes / 60, targetMinutes % 60)};
  FeedingScheduler scheduler;
  scheduler.setFeedTimes(times, 1);

  NextFeedInfo info = scheduler.computeNextFeed();
  // Should roll over to tomorrow: ~24h - 10min away, not "already happened".
  TEST_ASSERT_INT_WITHIN(1, 24 * 60 - 10, info.minutesUntil);
}

void test_compute_next_feed_picks_earliest_of_multiple_slots() {
  struct tm local;
  nowLocal(local);
  int nowMinutes = local.tm_hour * 60 + local.tm_min;
  int soon = (nowMinutes + 10) % (24 * 60);
  int later = (nowMinutes + 120) % (24 * 60);

  FeedTime times[] = {
    slot(later / 60, later % 60),
    slot(soon / 60, soon % 60),
  };
  FeedingScheduler scheduler;
  scheduler.setFeedTimes(times, 2);

  NextFeedInfo info = scheduler.computeNextFeed();
  TEST_ASSERT_INT_WITHIN(1, 10, info.minutesUntil);
  TEST_ASSERT_EQUAL(soon / 60, info.targetHour);
  TEST_ASSERT_EQUAL(soon % 60, info.targetMinute);
}

void test_compute_next_feed_wraps_a_full_week_for_missed_weekday_slot() {
  struct tm local;
  nowLocal(local);
  int nowMinutes = local.tm_hour * 60 + local.tm_min;
  int targetMinutes = ((nowMinutes - 5) % (24 * 60) + 24 * 60) % (24 * 60);

  // Same weekday as today, but time-of-day already passed -> wraps a week.
  FeedTime times[] = {slot(targetMinutes / 60, targetMinutes % 60, 1, local.tm_wday)};
  FeedingScheduler scheduler;
  scheduler.setFeedTimes(times, 1);

  NextFeedInfo info = scheduler.computeNextFeed();
  TEST_ASSERT_INT_WITHIN(1, 7 * 24 * 60 - 5, info.minutesUntil);
}

// ── cooldown: canFeedNow / getSecondsUntilManualFeedAllowed ──────────────────

void test_can_feed_now_true_before_any_feed_recorded() {
  FeedingScheduler scheduler;
  TEST_ASSERT_TRUE(scheduler.canFeedNow());
  TEST_ASSERT_EQUAL(0, scheduler.getSecondsUntilManualFeedAllowed());
}

void test_cooldown_blocks_immediately_after_manual_feed() {
  FeedingScheduler scheduler;
  scheduler.setMinFeedIntervalMinutes(5);
  scheduler.recordManualFeed();

  TEST_ASSERT_FALSE(scheduler.canFeedNow());
  int remaining = scheduler.getSecondsUntilManualFeedAllowed();
  // lastFedEpochMinute is truncated to whole minutes, so remaining can be
  // anywhere in (0, 300] depending on which second-of-minute this ran at.
  TEST_ASSERT_GREATER_THAN(0, remaining);
  TEST_ASSERT_LESS_OR_EQUAL(300, remaining);
}

void test_min_feed_interval_is_clamped_to_allowed_range() {
  FeedingScheduler scheduler;
  scheduler.setMinFeedIntervalMinutes(0);
  TEST_ASSERT_EQUAL(1, scheduler.getMinFeedIntervalMinutes());

  scheduler.setMinFeedIntervalMinutes(999999);
  TEST_ASSERT_EQUAL(1440, scheduler.getMinFeedIntervalMinutes());

  scheduler.setMinFeedIntervalMinutes(30);
  TEST_ASSERT_EQUAL(30, scheduler.getMinFeedIntervalMinutes());
}

// ── setFeedTimes: pure bookkeeping, deterministic ────────────────────────────

void test_set_feed_times_clamps_to_max_capacity() {
  FeedTime times[FeedingScheduler::MAX_FEED_TIMES + 5];
  for (int i = 0; i < FeedingScheduler::MAX_FEED_TIMES + 5; ++i) {
    times[i] = slot(i % 24, 0);
  }
  FeedingScheduler scheduler;
  scheduler.setFeedTimes(times, FeedingScheduler::MAX_FEED_TIMES + 5);
  TEST_ASSERT_EQUAL(FeedingScheduler::MAX_FEED_TIMES, scheduler.getFeedTimesCount());
}

// ── toLocalTime: fixed timestamps, fully deterministic ───────────────────────

void test_to_local_time_rejects_pre_2020_timestamps() {
  FeedingScheduler::setTimezoneOffsetMinutes(0);
  struct tm local;
  time_t year2000 = 946684800; // 2000-01-01T00:00:00Z
  TEST_ASSERT_FALSE(FeedingScheduler::toLocalTime(year2000, local));
}

void test_to_local_time_applies_timezone_offset() {
  FeedingScheduler::setTimezoneOffsetMinutes(120); // UTC+2
  struct tm local;
  time_t midnightUtc = 1735689600; // 2025-01-01T00:00:00Z
  TEST_ASSERT_TRUE(FeedingScheduler::toLocalTime(midnightUtc, local));
  TEST_ASSERT_EQUAL(2, local.tm_hour);
  FeedingScheduler::setTimezoneOffsetMinutes(FeedingScheduler::DEFAULT_TIMEZONE_OFFSET_MINUTES);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_compute_next_feed_returns_default_when_no_slots);
  RUN_TEST(test_compute_next_feed_picks_slot_later_today);
  RUN_TEST(test_compute_next_feed_wraps_to_tomorrow_when_slot_already_passed);
  RUN_TEST(test_compute_next_feed_picks_earliest_of_multiple_slots);
  RUN_TEST(test_compute_next_feed_wraps_a_full_week_for_missed_weekday_slot);
  RUN_TEST(test_can_feed_now_true_before_any_feed_recorded);
  RUN_TEST(test_cooldown_blocks_immediately_after_manual_feed);
  RUN_TEST(test_min_feed_interval_is_clamped_to_allowed_range);
  RUN_TEST(test_set_feed_times_clamps_to_max_capacity);
  RUN_TEST(test_to_local_time_rejects_pre_2020_timestamps);
  RUN_TEST(test_to_local_time_applies_timezone_offset);
  return UNITY_END();
}
