#include "oled_display.h"
#include <WiFi.h>
#include <Wire.h>
#include "feeding_scheduler.h"
#include "time.h"
#include "version_info.h"
#include <math.h>

OledDisplay::OledDisplay(int sdaPin, int sclPin)
  : u8g2(U8G2_R0, /* clock=*/ sclPin, /* data=*/ sdaPin, /* reset=*/ U8X8_PIN_NONE),
    sdaPin(sdaPin), sclPin(sclPin), lastUpdateTime(0), isInitialized(false),
    isPowerSaveEnabled(false), feedingStartTime(0) {
}

bool OledDisplay::checkDisplay() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  delay(50);
  return true;
}

void OledDisplay::begin() {
  isInitialized = false;
  
  Serial.print("OLED: Initializing Software I2C on SDA=");
  Serial.print(sdaPin);
  Serial.print(", SCL=");
  Serial.println(sclPin);
  
  Wire.begin(sdaPin, sclPin);

  Serial.println("OLED: Starting U8g2 Software I2C...");

  u8g2.begin();

  Serial.println("OLED: Configuring display...");

  u8g2.setFlipMode(0);
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);

  Serial.println("OLED: Testing display communication...");
  
  if (checkDisplay()) {
    isInitialized = true;
    Serial.println("OLED: ✓ Display initialized successfully!");
    
    showSplash();
    delay(1500);
    clear();
    Serial.println("OLED: Splash screen displayed");
  } else {
    Serial.println("OLED: ✗ Display test failed - continuing without OLED");
    isInitialized = false;
  }
}

void OledDisplay::showSplash() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(10, 22, "AquaFeed");
  u8g2.setFont(u8g2_font_ncenR08_tr);
  u8g2.drawStr(20, 40, "v" AQUAFEED_FIRMWARE_VERSION);
  u8g2.drawStr(20, 55, BUILD_DATE);
  u8g2.sendBuffer();
}

void OledDisplay::showDeepSleepNotice(bool scheduleTimerWakeEnabled) {
  if (!isInitialized) {
    delay(100);
    return;
  }
  u8g2.setPowerSave(0);
  isPowerSaveEnabled = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvR08_tr);
  u8g2.drawStr(0, 12, "Deep sleep");
  if (scheduleTimerWakeEnabled) {
    u8g2.drawStr(0, 28, "Wake: btn/timer");
  } else {
    u8g2.drawStr(0, 28, "Wake: press btn");
  }
  u8g2.drawStr(0, 44, "Web off in sleep");
  u8g2.sendBuffer();
  delay(2000);
}

void OledDisplay::clear() {
  if (!isInitialized) {
    return;
  }
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void OledDisplay::setPowerSave(bool enabled) {
  if (!isInitialized || isPowerSaveEnabled == enabled) {
    return;
  }

  isPowerSaveEnabled = enabled;
  u8g2.setPowerSave(enabled ? 1 : 0);
  if (enabled) {
    clear();
  } else {
    lastUpdateTime = 0;
  }
}

void OledDisplay::update(const DisplayData& data) {
  if (!isInitialized || isPowerSaveEnabled) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  static bool wasFeeding = false;
  bool isFeedingNow = data.isFeeding;
  
  if (!wasFeeding && isFeedingNow) {
    feedingStartTime = currentTime;
  }
  
  bool showFeedingScreen = (feedingStartTime > 0 && (currentTime - feedingStartTime) < FEEDING_DISPLAY_DURATION_MS);
  
  if (feedingStartTime > 0 && (currentTime - feedingStartTime) >= FEEDING_DISPLAY_DURATION_MS) {
    feedingStartTime = 0;
  }
  
  unsigned long updateInterval = showFeedingScreen ? 200 : UPDATE_INTERVAL_MS;

  bool forceUpdate = (showFeedingScreen && !wasFeeding) || (!showFeedingScreen && wasFeeding);
  wasFeeding = showFeedingScreen;

  if (!forceUpdate && currentTime - lastUpdateTime < updateInterval) {
    return;
  }
  lastUpdateTime = currentTime;

  u8g2.clearBuffer();

  if (showFeedingScreen) {
    drawFeedingScreen();
  } else {
    drawSimpleScreen(data);
  }

  u8g2.sendBuffer();
}

void OledDisplay::drawSimpleScreen(const DisplayData& data) {
  
  char timeBuf[16];
  formatTime(timeBuf, data.currentHour, data.currentMinute);
  char timeStr[24];
  snprintf(timeStr, sizeof(timeStr), "Time: %s", timeBuf);
  u8g2.setFont(u8g2_font_helvR10_tr);
  u8g2.drawStr(0, 13, timeStr);
  
  char batStr[24];
  if (data.isCharging) {
    snprintf(batStr, sizeof(batStr), "Bat: %.0f%% CHG", data.batteryPercent);
  } else {
    snprintf(batStr, sizeof(batStr), "Bat: %.0f%%", data.batteryPercent);
  }
  u8g2.setFont(u8g2_font_helvR10_tr);
  u8g2.drawStr(0, 28, batStr);
  
  char nextStr[32];
  if (data.nextFeedHour >= 0 && data.nextFeedMinute >= 0 && data.minutesUntilNext > 0) {
    time_t now = time(nullptr);
    struct tm localTime;
    int remainingSeconds = 0;
    
    if (FeedingScheduler::toLocalTime(now, localTime) && localTime.tm_year + 1900 >= 2020) {
      int currentTotalSeconds = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;
      int nextTotalSeconds = data.nextFeedHour * 3600 + data.nextFeedMinute * 60;
      
      int diffSeconds = nextTotalSeconds - currentTotalSeconds;
      if (diffSeconds <= 0) {
        diffSeconds += 24 * 3600;
      }
      remainingSeconds = diffSeconds;
    } else {
      remainingSeconds = data.minutesUntilNext * 60;
    }
    
    int hours = remainingSeconds / 3600;
    int mins = (remainingSeconds % 3600) / 60;
    int secs = remainingSeconds % 60;
    
    if (hours > 0) {
      snprintf(nextStr, sizeof(nextStr), "Next: %dh %dm %ds", hours, mins, secs);
    } else if (mins > 0) {
      snprintf(nextStr, sizeof(nextStr), "Next: %dm %ds", mins, secs);
    } else {
      snprintf(nextStr, sizeof(nextStr), "Next: %ds", secs);
    }
  } else {
    snprintf(nextStr, sizeof(nextStr), "Next: --:--");
  }
  u8g2.setFont(u8g2_font_helvR10_tr);
  if (data.cooldownSeconds > 0) {
    char coolStr[32];
    int cm = data.cooldownSeconds / 60;
    int cs = data.cooldownSeconds % 60;
    if (cm > 0) {
      snprintf(coolStr, sizeof(coolStr), "Wait: %dm %ds", cm, cs);
    } else {
      snprintf(coolStr, sizeof(coolStr), "Wait: %ds", cs);
    }
    u8g2.drawStr(0, 43, coolStr);
  } else {
    u8g2.drawStr(0, 43, nextStr);
  }

  if (data.ip[0] != '\0') {
    u8g2.drawStr(0, 58, data.ip);
  }
}

void OledDisplay::drawFeedingScreen() {
  unsigned long currentTime = millis();
  
  int spinnerPhase = (currentTime / 200) % 8;
  
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.drawStr(20, 28, "Feeding");
  
  int centerX = 64;
  int centerY = 50;
  int radius = 10;
  
  for (int i = 0; i < 8; i++) {
    float angle = (i * 3.14159f * 2.0f) / 8.0f;
    int x = centerX + cosf(angle) * radius;
    int y = centerY + sinf(angle) * radius;
    
    if (i == spinnerPhase) {
      u8g2.drawDisc(x, y, 3, U8G2_DRAW_ALL);
    } else {
      u8g2.drawDisc(x, y, 1, U8G2_DRAW_ALL);
    }
  }
}

void OledDisplay::formatTime(char* buffer, int hour, int minute) {
  if (hour < 0 || minute < 0) {
    snprintf(buffer, 16, "--:--");
  } else {
    snprintf(buffer, 16, "%02d:%02d", hour, minute);
  }
}

