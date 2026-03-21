#include "servo_controller.h"

ServoController::ServoController(int pin, int minAngle, int maxAngle)
  : pin(pin), minAngle(minAngle), maxAngle(maxAngle),
    speedSetting(20.0), currentAngle(0), manualMoving(false) {
}

void ServoController::begin() {
  servo.setPeriodHertz(50);
  servo.attach(pin, 600, 2400);
  servo.write(currentAngle);
}

void ServoController::setAngle(int angle, bool smooth) {
  angle = constrain(angle, minAngle, maxAngle);
  if (smooth) {
    moveServoSmooth(angle);
  } else {
    moveServoFast(angle);
  }
}

void ServoController::setSpeed(float speed) {
  speedSetting = speed;
}

int ServoController::speedToStepDelayMs(float sliderSpeed) {
  float minSpeed = 19.5;
  float maxSpeed = 20.0;
  float normalized = (sliderSpeed - 1) / (20 - 1);
  float realSpeed = minSpeed + normalized * (maxSpeed - minSpeed);
  int stepDelay = (int)((20.0 - realSpeed) * 10);
  return max(stepDelay, 0);
}

void ServoController::moveServoSmooth(int target) {
  target = constrain(target, minAngle, maxAngle);
  if (target == currentAngle) return;
  int stepDelay = speedToStepDelayMs(speedSetting);
  if (target > currentAngle) {
    for (int a = currentAngle + 1; a <= target; ++a) {
      servo.write(a);
      delay(stepDelay);
    }
  } else {
    for (int a = currentAngle - 1; a >= target; --a) {
      servo.write(a);
      delay(stepDelay);
    }
  }
  currentAngle = target;
}

void ServoController::moveServoFast(int target) {
  target = constrain(target, minAngle, maxAngle);
  servo.write(target);
  currentAngle = target;
}

void ServoController::feedSequence(int repeats) {
  manualMoving = true;
  for (int i = 0; i < repeats; i++) {
    moveServoSmooth(minAngle);
    delay(50);
    moveServoSmooth(maxAngle);
    delay(50);
    moveServoSmooth(minAngle);
    delay(50);
  }
  manualMoving = false;
}

