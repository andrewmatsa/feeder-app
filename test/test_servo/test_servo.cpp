#include <unity.h>
#include "../mock_arduino.h"

// Inline the fixed speedToStepDelayMs logic (mirrors src/servo_controller.cpp).
// map(constrain(speed, 1, 20), 1, 20, 200, 2):
//   speed=1  → 200ms  (slow)
//   speed=20 →   2ms  (fast)
static int speedToStepDelayMs(float sliderSpeed) {
    return (int)map(constrain((int)sliderSpeed, 1, 20), 1, 20, 200, 2);
}

void setUp() {}
void tearDown() {}

// ── Test cases ─────────────────────────────────────────────────────────────

void test_speed1_is_slowest() {
    TEST_ASSERT_EQUAL(200, speedToStepDelayMs(1.0f));
}

void test_speed20_is_fastest() {
    TEST_ASSERT_EQUAL(2, speedToStepDelayMs(20.0f));
}

void test_speed10_is_midrange() {
    int d = speedToStepDelayMs(10.0f);
    // map(10, 1, 20, 200, 2): (10-1)*(2-200)/(20-1)+200 = 9*(-198)/19+200 = -93+200 = 107
    TEST_ASSERT_EQUAL(107, d);
}

void test_speed_below1_clamps_to_200() {
    TEST_ASSERT_EQUAL(200, speedToStepDelayMs(0.0f));
    TEST_ASSERT_EQUAL(200, speedToStepDelayMs(-5.0f));
}

void test_speed_above20_clamps_to_2() {
    TEST_ASSERT_EQUAL(2, speedToStepDelayMs(21.0f));
    TEST_ASSERT_EQUAL(2, speedToStepDelayMs(100.0f));
}

void test_speed_is_monotonically_decreasing() {
    // Higher speed value → smaller delay (faster movement)
    TEST_ASSERT_GREATER_OR_EQUAL(speedToStepDelayMs(10.0f), speedToStepDelayMs(5.0f));
    TEST_ASSERT_GREATER_OR_EQUAL(speedToStepDelayMs(15.0f), speedToStepDelayMs(10.0f));
    TEST_ASSERT_GREATER_OR_EQUAL(speedToStepDelayMs(20.0f), speedToStepDelayMs(15.0f));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_speed1_is_slowest);
    RUN_TEST(test_speed20_is_fastest);
    RUN_TEST(test_speed10_is_midrange);
    RUN_TEST(test_speed_below1_clamps_to_200);
    RUN_TEST(test_speed_above20_clamps_to_2);
    RUN_TEST(test_speed_is_monotonically_decreasing);
    return UNITY_END();
}
