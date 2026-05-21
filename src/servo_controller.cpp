#include "servo_controller.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
  return map(constrain((int)sliderSpeed, 10, 20), 10, 20, 30, 5);
}

void ServoController::moveServoSmooth(int target) {
  target = constrain(target, minAngle, maxAngle);
  if (target == currentAngle) return;
  ensureAttached();
  int stepDelay = speedToStepDelayMs(speedSetting);
  // Step size: speed=1 → 1°, speed=20 → 5° per write
  int stepSize = map(constrain((int)speedSetting, 10, 20), 10, 20, 1, 5);
  if (target > currentAngle) {
    for (int a = currentAngle + stepSize; a <= target; a += stepSize) {
      servo.write(min(a, target));
      delay(stepDelay);
    }
  } else {
    for (int a = currentAngle - stepSize; a >= target; a -= stepSize) {
      servo.write(max(a, target));
      delay(stepDelay);
    }
  }
  servo.write(target);
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

void ServoController::feedTask(void* params) {
  auto* p = static_cast<FeedTaskParams*>(params);
  p->self->feedSequence(p->repeats);
  if (p->onDone) p->onDone();
  delete p;
  vTaskDelete(NULL);
}

void ServoController::feedSequenceAsync(int repeats, std::function<void()> onDone) {
  if (manualMoving) return;
  auto* params = new FeedTaskParams{this, repeats, onDone};
  xTaskCreatePinnedToCore(feedTask, "feed", 4096, params, 1, NULL, 0);
}

