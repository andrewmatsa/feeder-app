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
// True until the backend confirms this device's MAC is claimed by an account
// (see backend/devices.py's register-ip response). Starts true so we never
// flash a false "not added" screen before the first successful backend reply.
extern bool isDeviceClaimed;
// WiFi-provisioning progress (AP-mode "connect to home WiFi" flow) — used by
// the OLED to show live connecting/connected/failed feedback instead of a
// static "join FishFeeder-XXXX" screen for the whole duration.
const char* getProvisionStateStr();
String getProvisionTargetSsid();
void recheckClaimStatusIfNeeded();

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

