#include <unity.h>
#include "../mock_arduino.h"
#include <cmath>

// Mirrors BatteryMonitor::voltageToPercent from src/battery_monitor.cpp
static const float MAX_VOLTAGE = 8.4f;
static const float MIN_VOLTAGE = 6.0f;

static float voltageToPercent(float v) {
    if (v >= MAX_VOLTAGE) return 100.0f;
    if (v <= MIN_VOLTAGE) return 0.0f;
    float percent = (v - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return percent;
}

void setUp() {}
void tearDown() {}

// ── Test cases ─────────────────────────────────────────────────────────────

void test_min_voltage_gives_0_percent() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltageToPercent(6.0f));
}

void test_below_min_clamps_to_0() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltageToPercent(5.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltageToPercent(0.0f));
}

void test_max_voltage_gives_100_percent() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, voltageToPercent(8.4f));
}

void test_above_max_clamps_to_100() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, voltageToPercent(9.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, voltageToPercent(12.0f));
}

void test_midpoint_voltage_is_50_percent() {
    float mid = (MAX_VOLTAGE + MIN_VOLTAGE) / 2.0f;  // 7.2V
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 50.0f, voltageToPercent(mid));
}

void test_quarter_voltage_is_25_percent() {
    float quarter = MIN_VOLTAGE + (MAX_VOLTAGE - MIN_VOLTAGE) * 0.25f;  // 6.6V
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 25.0f, voltageToPercent(quarter));
}

void test_percent_increases_with_voltage() {
    TEST_ASSERT_GREATER_THAN(voltageToPercent(6.5f), voltageToPercent(7.0f));
    TEST_ASSERT_GREATER_THAN(voltageToPercent(7.0f), voltageToPercent(7.5f));
    TEST_ASSERT_GREATER_THAN(voltageToPercent(7.5f), voltageToPercent(8.0f));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_min_voltage_gives_0_percent);
    RUN_TEST(test_below_min_clamps_to_0);
    RUN_TEST(test_max_voltage_gives_100_percent);
    RUN_TEST(test_above_max_clamps_to_100);
    RUN_TEST(test_midpoint_voltage_is_50_percent);
    RUN_TEST(test_quarter_voltage_is_25_percent);
    RUN_TEST(test_percent_increases_with_voltage);
    return UNITY_END();
}
