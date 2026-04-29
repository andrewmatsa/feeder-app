#include "battery_monitor.h"
#include <Arduino.h>
#include <math.h>

const float BatteryMonitor::ADC_REFERENCE_VOLTAGE = 3.3f;
const float BatteryMonitor::ADC_MAX_VALUE = 4095.0f;
const float BatteryMonitor::VOLTAGE_DIVIDER_RATIO = 5.08f;
const float BatteryMonitor::DEFAULT_BATTERY_CALIBRATION = 0.518f;
const float BatteryMonitor::MIN_CALIBRATION_FACTOR = 0.1f;
const float BatteryMonitor::MAX_CALIBRATION_FACTOR = 5.0f;
const float BatteryMonitor::MAX_VOLTAGE = 8.4f;
// 2S Li-ion practical lower bound for UI percentage mapping.
const float BatteryMonitor::MIN_VOLTAGE = 6.0f;

BatteryMonitor::BatteryMonitor(int pin)
  : pin(pin), batteryVoltage(0.0), batteryPercent(0.0), calibrationFactor(DEFAULT_BATTERY_CALIBRATION) {
}

void BatteryMonitor::begin() {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
#endif
  pinMode(pin, INPUT);
}

float BatteryMonitor::readVoltage() {
  uint32_t accumulator = 0;
  for (int i = 0; i < BATTERY_SAMPLES; ++i) {
    accumulator += analogRead(pin);
    delayMicroseconds(200);
  }
  float raw = accumulator / static_cast<float>(BATTERY_SAMPLES);
  float voltage = (raw / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE * VOLTAGE_DIVIDER_RATIO;
  voltage *= calibrationFactor;
  return voltage;
}

void BatteryMonitor::setCalibrationFactor(float factor) {
  if (!isfinite(factor)) {
    return;
  }
  if (factor < MIN_CALIBRATION_FACTOR) {
    factor = MIN_CALIBRATION_FACTOR;
  }
  if (factor > MAX_CALIBRATION_FACTOR) {
    factor = MAX_CALIBRATION_FACTOR;
  }
  calibrationFactor = factor;
}

float BatteryMonitor::voltageToPercent(float v) {
  if (v >= MAX_VOLTAGE) return 100.0f;
  if (v <= MIN_VOLTAGE) return 0.0f;
  
  float percent = (v - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0f;
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  return percent;
}

void BatteryMonitor::update() {
  batteryVoltage = readVoltage();
  batteryPercent = voltageToPercent(batteryVoltage);
}

