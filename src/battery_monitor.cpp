#include "battery_monitor.h"
#include <Arduino.h>

const float BatteryMonitor::ADC_REFERENCE_VOLTAGE = 3.3f;
const float BatteryMonitor::ADC_MAX_VALUE = 4095.0f;
const float BatteryMonitor::VOLTAGE_DIVIDER_RATIO = 5.08f;
const float BatteryMonitor::BATTERY_CALIBRATION = 0.58f;
const float BatteryMonitor::MAX_VOLTAGE = 8.4f;
const float BatteryMonitor::MIN_VOLTAGE = 6.6f;

BatteryMonitor::BatteryMonitor(int pin)
  : pin(pin), batteryVoltage(0.0), batteryPercent(0.0) {
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
  voltage *= BATTERY_CALIBRATION;
  return voltage;
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

