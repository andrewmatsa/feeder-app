#pragma once
#include <WebServer.h>
#include "battery_monitor.h"

class OtaHandler {
public:
  OtaHandler(WebServer& server, BatteryMonitor& battery);
  void registerRoutes();

private:
  WebServer& server;
  BatteryMonitor& battery;
  bool otaError = false;
  bool batteryTooLow = false;

  void handleOtaUpload();
  void handleOtaUploadBody();
};
