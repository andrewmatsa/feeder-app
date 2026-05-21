#include "ota_handler.h"
#include <Update.h>
#include "wifi_manager.h"

static constexpr int OTA_MIN_BATTERY_PERCENT = 20;

OtaHandler::OtaHandler(WebServer& server, BatteryMonitor& battery)
  : server(server), battery(battery) {}

void OtaHandler::registerRoutes() {
  server.on(
    "/api/otaUpdate",
    HTTP_POST,
    [this]() { handleOtaUpload(); },
    [this]() { handleOtaUploadBody(); }
  );
}

void OtaHandler::handleOtaUploadBody() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    otaError = false;
    batteryTooLow = false;

    if (!isTrustedMutationRequest(server)) {
      Serial.println("[OTA] Rejected: unauthorized");
      otaError = true;
      return;
    }

    battery.update();
    if (battery.getPercent() < OTA_MIN_BATTERY_PERCENT) {
      Serial.printf("[OTA] Rejected: battery too low (%d%%)\n", battery.getPercent());
      otaError = true;
      batteryTooLow = true;
      return;
    }

    Serial.printf("[OTA] Start: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
      otaError = true;
    }
  }

  if (upload.status == UPLOAD_FILE_WRITE && !otaError) {
    size_t written = Update.write(upload.buf, upload.currentSize);
    if (written != upload.currentSize) {
      Update.printError(Serial);
      otaError = true;
    }
    Serial.printf("[OTA] Written %u / %u bytes\n", upload.totalSize, upload.totalSize);
  }

  if (upload.status == UPLOAD_FILE_END) {
    if (!otaError) {
      if (!Update.end(true)) {
        Update.printError(Serial);
        otaError = true;
      } else {
        Serial.println("[OTA] Done, rebooting...");
      }
    }
  }

  if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    otaError = true;
    Serial.println("[OTA] Aborted");
  }
}

void OtaHandler::handleOtaUpload() {
  if (otaError) {
    if (batteryTooLow) {
      server.send(412, "application/json",
        "{\"error\":\"battery_too_low\",\"batteryPercent\":" +
        String(battery.getPercent()) + "}");
    } else {
      server.send(500, "application/json",
        "{\"error\":\"flash_failed\",\"message\":\"" +
        String(Update.errorString()) + "\"}");
    }
    return;
  }
  server.send(200, "application/json", "{\"status\":\"rebooting\"}");
  delay(500);
  ESP.restart();
}
