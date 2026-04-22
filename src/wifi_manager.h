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
extern String apSSID;
extern String apPassword;
extern bool isAPMode;

// === WiFi Management Functions ===
bool connectToWiFi();
void startAPMode();
void initWiFi(Preferences& preferences);
void setupWiFiHandlers(WebServer& server, Preferences& preferences);
void configureRequestSecurity(WebServer& server);
bool isTrustedMutationRequest(WebServer& server);

// === WiFi HTML Page ===
extern const char* pageWiFi;

// === WiFi Handlers ===
void handleWiFi(WebServer& server);
void handleSetWiFi(WebServer& server, Preferences& preferences);
void handleForgetWiFi(WebServer& server, Preferences& preferences);
void handleReconnectWiFi(WebServer& server);

#endif

