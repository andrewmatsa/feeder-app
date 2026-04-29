#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

class BatteryMonitor {
public:
  BatteryMonitor(int pin);
  void begin();
  float readVoltage();
  void setCalibrationFactor(float factor);
  float getCalibrationFactor() const { return calibrationFactor; }
  float getVoltage() const { return batteryVoltage; }
  float getPercent() const { return batteryPercent; }
  bool isCharging() const { return chargingLikely; }
  void update();
  
private:
  int pin;
  float batteryVoltage;
  float batteryPercent;
  float calibrationFactor;
  bool chargingLikely;
  bool trendInitialized;
  float trendVoltage;
  unsigned long lastTrendSampleMs;
  int risingTrendSamples;
  int fallingTrendSamples;
  
  static const int BATTERY_SAMPLES = 16;
  static const float ADC_REFERENCE_VOLTAGE;
  static const float ADC_MAX_VALUE;
  static const float VOLTAGE_DIVIDER_RATIO;
  static const float DEFAULT_BATTERY_CALIBRATION;
  static const float MIN_CALIBRATION_FACTOR;
  static const float MAX_CALIBRATION_FACTOR;
  static const float MAX_VOLTAGE;
  static const float MIN_VOLTAGE;
  static const float CHARGING_RISE_THRESHOLD_V_PER_MIN;
  static const float CHARGING_FALL_THRESHOLD_V_PER_MIN;
  static const int CHARGING_RISE_SAMPLES_REQUIRED;
  static const int CHARGING_FALL_SAMPLES_REQUIRED;
  
  float voltageToPercent(float v);
};

#endif

