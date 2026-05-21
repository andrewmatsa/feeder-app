#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <ESP32Servo.h>
#include <functional>

class ServoController {
public:
  ServoController(int pin, int minAngle = 0, int maxAngle = 180);
  void begin();
  void setAngle(int angle, bool smooth = false);
  void setSpeed(float speed);
  void setPowerSave(bool enabled);
  float getSpeed() const { return speedSetting; }
  int getCurrentAngle() const { return currentAngle; }
  bool isMoving() const { return manualMoving; }
  void setMoving(bool moving) { manualMoving = moving; }

  void feedSequence(int repeats = 1);
  void feedSequenceAsync(int repeats, std::function<void()> onDone = nullptr);

private:
  Servo servo;
  int pin;
  int minAngle;
  int maxAngle;
  float speedSetting;
  int currentAngle;
  bool manualMoving;
  bool servoAttached;
  
  void ensureAttached();
  void moveServoSmooth(int target);
  void moveServoFast(int target);
  int speedToStepDelayMs(float sliderSpeed);

  struct FeedTaskParams {
    ServoController* self;
    int repeats;
    std::function<void()> onDone;
  };
  static void feedTask(void* params);
};

#endif

