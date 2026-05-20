#include "servo_controller.h"

ServoController::ServoController(int pin, int minAngle, int maxAngle)
  : pin(pin), minAngle(minAngle), maxAngle(maxAngle),
    speedSetting(20.0), currentAngle(0), manualMoving(false), servoAttached(false) {
}

void ServoController::begin() {
  ensureAttached();
  servo.write(currentAngle);
}

void ServoController::ensureAttached() {
  if (servoAttached) {
    return;
  }
  servo.setPeriodHertz(50);
  servo.attach(pin, 600, 2400);
  servo.write(currentAngle);
  delay(20);
  servoAttached = true;
}

void ServoController::setPowerSave(bool enabled) {
  if (enabled) {
    if (servoAttached && !manualMoving) {
      servo.detach();
      servoAttached = false;
    }
    return;
  }

  ensureAttached();
}

void ServoController::setAngle(int angle, bool smooth) {
  angle = constrain(angle, minAngle, maxAngle);
  ensureAttached();
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
  return map(constrain((int)sliderSpeed, 1, 20), 1, 20, 200, 2);
}

void ServoController::moveServoSmooth(int target) {
  target = constrain(target, minAngle, maxAngle);
  if (target == currentAngle) return;
  ensureAttached();
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
  ensureAttached();
  servo.write(target);
  currentAngle = target;
}

void ServoController::feedSequence(int repeats) {
  manualMoving = true;
  ensureAttached();
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

