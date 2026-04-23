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
bool isApSessionAuthorized(WebServer& server);
void handleApLogin(WebServer& server);
void handleApLoginStatus(WebServer& server);
void handleProvisionWiFi(WebServer& server, Preferences& preferences);
void handleProvisionWiFiStatus(WebServer& server);
void updateWiFiProvisioning();

// === WiFi HTML Page ===
extern const char* pageWiFi;
extern const char* pageWiFiLocked;
extern const char* pageWiFiConnect;

// === WiFi Handlers ===
void handleWiFi(WebServer& server);
void handleSetWiFi(WebServer& server, Preferences& preferences);
void handleForgetWiFi(WebServer& server, Preferences& preferences);
void handleReconnectWiFi(WebServer& server);

#endif

