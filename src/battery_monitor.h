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
  void update();
  
private:
  int pin;
  float batteryVoltage;
  float batteryPercent;
  float calibrationFactor;
  
  static const int BATTERY_SAMPLES = 16;
  static const float ADC_REFERENCE_VOLTAGE;
  static const float ADC_MAX_VALUE;
  static const float VOLTAGE_DIVIDER_RATIO;
  static const float DEFAULT_BATTERY_CALIBRATION;
  static const float MIN_CALIBRATION_FACTOR;
  static const float MAX_CALIBRATION_FACTOR;
  static const float MAX_VOLTAGE;
  static const float MIN_VOLTAGE;
  
  float voltageToPercent(float v);
};

#endif

