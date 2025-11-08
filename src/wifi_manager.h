#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include "time.h"

// === WiFi Variables ===
extern String savedSSID;
extern String savedPassword;
extern const char* apSSID;
extern const char* apPassword;
extern bool isAPMode;

// === WiFi Management Functions ===
bool connectToWiFi();
void startAPMode();
void initWiFi(Preferences& preferences);
void setupWiFiHandlers(WebServer& server, Preferences& preferences);

// === WiFi HTML Page ===
extern const char* pageWiFi;

// === WiFi Handlers ===
void handleWiFi(WebServer& server);
void handleSetWiFi(WebServer& server, Preferences& preferences);
void handleScanWiFi(WebServer& server);
void handleReconnectWiFi(WebServer& server);

#endif

