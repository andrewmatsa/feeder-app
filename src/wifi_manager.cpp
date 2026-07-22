#include "wifi_manager.h"
#include <Arduino.h>
#include <esp_system.h>

// === WiFi Variables ===
String savedSSID = "";
String savedPassword = "";
String apSSID = "FishFeeder-Setup";
String apPassword = "";
bool isAPMode = false;

namespace {
const char* kMutationClientHeader = "X-AquaFeed-Client";
const char* kApSessionHeader = "X-AquaFeed-Session";
const char* kCookieHeader = "Cookie";
const char* kWebUiClient = "webui";
const char* kBackendClient = "backend";
const char* kDefaultApPassword = "12345678";
const char* kSessionCookieName = "AquaFeedSession";
constexpr unsigned long PROVISION_RESPONSE_GRACE_MS = 8000UL;
constexpr unsigned long PROVISION_CONNECT_TIMEOUT_MS = 20000UL;
String apSessionToken = "";
Preferences* wifiPreferencesRef = nullptr;

enum class ProvisionState {
  Idle,
  Pending,
  Connecting,
  Connected,
  Failed,
};

ProvisionState provisionState = ProvisionState::Idle;
String provisionTargetSsid = "";
String provisionTargetPassword = "";
String provisionPreviousSsid = "";
String provisionPreviousPassword = "";
String provisionStationIp = "";
unsigned long provisionStartedAtMs = 0;
unsigned long provisionConnectedAtMs = 0;

const char* provisionStateToString() {
  switch (provisionState) {
    case ProvisionState::Pending: return "pending";
    case ProvisionState::Connecting: return "connecting";
    case ProvisionState::Connected: return "connected";
    case ProvisionState::Failed: return "failed";
    default: return "idle";
  }
}

String buildApSsid() {
  char buffer[24];
  unsigned long chipSuffix = static_cast<unsigned long>(ESP.getEfuseMac() & 0xFFFF);
  snprintf(buffer, sizeof(buffer), "FishFeeder-%04lX", chipSuffix);
  return String(buffer);
}

void ensureApCredentials(Preferences& preferences) {
  apSSID = buildApSsid();
  apPassword = preferences.getString("apPassword", "");
  if (apPassword != kDefaultApPassword) {
    apPassword = kDefaultApPassword;
    preferences.putString("apPassword", apPassword);
  }
}

void restartStationServices() {
  if (!MDNS.begin("fish")) Serial.println("Error setting up MDNS!");
  else Serial.println("mDNS responder started: http://fish.local");
  configTime(0, 0, "pool.ntp.org", "time.google.com");
}

String createApSessionToken() {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%08lX%08lX",
           static_cast<unsigned long>(esp_random()),
           static_cast<unsigned long>(esp_random()));
  return String(buffer);
}

void clearApSession() {
  apSessionToken = "";
}

void activateApMode(bool clearSession) {
  Serial.println("Starting Access Point mode...");
  if (clearSession) {
    clearApSession();
  }
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  IPAddress IP = WiFi.softAPIP();
  ets_printf("[WIFI] AP mode: SSID=%s IP=%s\n", apSSID.c_str(), IP.toString().c_str());
  isAPMode = true;
}

bool hasValidApSession(WebServer& server) {
  if (apSessionToken.length() == 0) {
    return false;
  }
  if (server.hasHeader(kApSessionHeader) && server.header(kApSessionHeader) == apSessionToken) {
    return true;
  }
  if (server.hasHeader(kCookieHeader)) {
    const String cookie = server.header(kCookieHeader);
    const String expected = String(kSessionCookieName) + "=" + apSessionToken;
    return cookie.indexOf(expected) >= 0;
  }
  return false;
}

bool connectToWiFiInternal(bool keepApAlive) {
  if(savedSSID.length() == 0) return false;

  WiFi.mode(keepApAlive ? WIFI_AP_STA : WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  Serial.print("Connecting to WiFi: " + savedSSID);

  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    ets_printf("\n[WIFI] Connected, IP: %s\n", WiFi.localIP().toString().c_str());
    if (!keepApAlive) {
      isAPMode = false;
    }
    return true;
  }

  Serial.println("\nFailed to connect to WiFi");
  return false;
}
}  // namespace

void configureRequestSecurity(WebServer& server) {
  const char* headerKeys[] = {kMutationClientHeader, kApSessionHeader, kCookieHeader, "Accept"};
  server.collectHeaders(headerKeys, 4);
}

bool isApSessionAuthorized(WebServer& server) {
  return !isAPMode || hasValidApSession(server);
}

bool isTrustedMutationRequest(WebServer& server) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "use POST");
    return false;
  }
  if (!server.hasHeader(kMutationClientHeader)) {
    server.send(403, "text/plain", "missing client header");
    return false;
  }
  const String client = server.header(kMutationClientHeader);
  if (client == kWebUiClient || client == kBackendClient) {
    if (!isApSessionAuthorized(server)) {
      server.send(403, "text/plain", "login required");
      return false;
    }
    return true;
  }
  server.send(403, "text/plain", "untrusted client");
  return false;
}

// === WiFi Management Functions ===
bool connectToWiFi() {
  return connectToWiFiInternal(false);
}

void startAPMode() {
  activateApMode(true);
}

void initWiFi(Preferences& preferences) {
  wifiPreferencesRef = &preferences;
  ensureApCredentials(preferences);
  // Завантажуємо збережені WiFi дані
  savedSSID = preferences.getString("wifiSSID", "");
  savedPassword = preferences.getString("wifiPassword", "");

  // Підключаємося до WiFi
  if(!connectToWiFi()) {
    // Якщо не вдалося підключитися, увімкнути AP mode
    startAPMode();
  }
  
  if(!isAPMode) {
    restartStationServices();
  }
}

void setupWiFiHandlers(WebServer& server, Preferences& preferences) {
  configureRequestSecurity(server);
  server.on("/wifi", HTTP_GET, [&server](){ handleWiFi(server); });
  server.on("/api/apLogin", HTTP_POST, [&server](){ handleApLogin(server); });
  server.on("/api/apLoginStatus", HTTP_GET, [&server](){ handleApLoginStatus(server); });
  server.on("/api/provisionWiFi", HTTP_POST, [&server, &preferences](){ handleProvisionWiFi(server, preferences); });
  server.on("/api/provisionWiFiStatus", HTTP_GET, [&server](){ handleProvisionWiFiStatus(server); });
  server.on("/api/setWiFi", HTTP_POST, [&server, &preferences](){ handleSetWiFi(server, preferences); });
  server.on("/api/forgetWiFi", HTTP_POST, [&server, &preferences](){ handleForgetWiFi(server, preferences); });
  server.on("/api/reconnectWiFi", HTTP_POST, [&server](){ handleReconnectWiFi(server); });
}

// WiFi page definition moved to src/web/wifi_page.cpp.

// === WiFi Handlers ===
void handleWiFi(WebServer& server) {
  if (isAPMode && !hasValidApSession(server)) {
    server.send_P(200, "text/html", pageWiFiLocked);
    return;
  }
  if (isAPMode) {
    server.send_P(200, "text/html", pageWiFiConnect);
    return;
  }
  server.send_P(200, "text/html", pageWiFi);
}

void handleApLogin(WebServer& server) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "use POST");
    return;
  }
  if (!server.hasHeader(kMutationClientHeader) || server.header(kMutationClientHeader) != kWebUiClient) {
    server.send(403, "text/plain", "untrusted client");
    return;
  }
  if (!isAPMode) {
    server.send(200, "application/json", "{\"ok\":true,\"requiresLogin\":false,\"authenticated\":true}");
    return;
  }
  if (!server.hasArg("password")) {
    server.send(400, "text/plain", "missing password");
    return;
  }
  if (server.arg("password") != apPassword) {
    clearApSession();
    server.send(403, "text/plain", "invalid password");
    return;
  }

  apSessionToken = createApSessionToken();
  server.sendHeader("Set-Cookie", String(kSessionCookieName) + "=" + apSessionToken + "; Path=/");
  String response = "{\"ok\":true,\"requiresLogin\":true,\"authenticated\":true,\"token\":\"" + apSessionToken + "\"}";
  server.send(200, "application/json", response);
}

void handleApLoginStatus(WebServer& server) {
  if (server.method() != HTTP_GET) {
    server.send(405, "text/plain", "use GET");
    return;
  }

  const bool authenticated = !isAPMode || hasValidApSession(server);
  String response = String("{\"requiresLogin\":") + (isAPMode ? "true" : "false") +
                    ",\"authenticated\":" + (authenticated ? "true" : "false") + "}";
  server.send(200, "application/json", response);
}

void handleProvisionWiFi(WebServer& server, Preferences& preferences) {
  if (!isTrustedMutationRequest(server)) return;
  if (!isAPMode) {
    server.send(409, "application/json", "{\"ok\":false,\"message\":\"device already connected\"}");
    return;
  }
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"missing ssid or password\"}");
    return;
  }

  const String previousSSID = savedSSID;
  const String previousPassword = savedPassword;
  provisionPreviousSsid = previousSSID;
  provisionPreviousPassword = previousPassword;
  provisionTargetSsid = server.arg("ssid");
  provisionTargetPassword = server.arg("password");
  provisionStationIp = "";
  provisionStartedAtMs = 0;
  provisionConnectedAtMs = 0;
  provisionState = ProvisionState::Pending;
  String response = "{\"ok\":true,\"connecting\":true,\"status\":\"pending\",\"ssid\":\"" + provisionTargetSsid + "\"}";
  server.send(202, "application/json", response);
}

void handleProvisionWiFiStatus(WebServer& server) {
  if (isAPMode && !hasValidApSession(server)) {
    server.send(403, "text/plain", "login required");
    return;
  }
  String response = "{\"ok\":true,\"status\":\"" + String(provisionStateToString()) +
                    "\",\"ssid\":\"" + provisionTargetSsid +
                    "\",\"ip\":\"" + provisionStationIp +
                    "\",\"url\":\"http://fish.local/\"}";
  if (provisionState == ProvisionState::Failed) {
    response += ",\"message\":\"wifi connection failed\"";
  }
  response += "}";
  server.send(200, "application/json", response);
}

void updateWiFiProvisioning() {
  if (provisionState == ProvisionState::Idle) {
    return;
  }

  if (provisionState == ProvisionState::Pending) {
    savedSSID = provisionTargetSsid;
    savedPassword = provisionTargetPassword;
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.disconnect(false, false);
    delay(50);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    provisionStartedAtMs = millis();
    provisionState = ProvisionState::Connecting;
    Serial.println("Provisioning: started background WiFi connection");
    return;
  }

  if (provisionState == ProvisionState::Connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      provisionStationIp = WiFi.localIP().toString();
      provisionConnectedAtMs = millis();
      provisionState = ProvisionState::Connected;
      if (wifiPreferencesRef) {
        wifiPreferencesRef->putString("wifiSSID", savedSSID);
        wifiPreferencesRef->putString("wifiPassword", savedPassword);
      }
      Serial.println("Provisioning: WiFi connected, keeping AP alive for user notification");
      return;
    }

    if ((millis() - provisionStartedAtMs) >= PROVISION_CONNECT_TIMEOUT_MS) {
      savedSSID = provisionPreviousSsid;
      savedPassword = provisionPreviousPassword;
      provisionStationIp = "";
      provisionState = ProvisionState::Failed;
      WiFi.disconnect(false, false);
      activateApMode(false);
      Serial.println("Provisioning: WiFi connection failed");
    }
    return;
  }

  if (provisionState == ProvisionState::Connected) {
    if ((millis() - provisionConnectedAtMs) < PROVISION_RESPONSE_GRACE_MS) {
      return;
    }
    WiFi.softAPdisconnect(true);
    isAPMode = false;
    clearApSession();
    restartStationServices();
    provisionState = ProvisionState::Idle;
    provisionTargetSsid = "";
    provisionTargetPassword = "";
    provisionPreviousSsid = "";
    provisionPreviousPassword = "";
    provisionStationIp = "";
    Serial.println("Provisioning: AP mode disabled after successful handoff");
    return;
  }

  if (provisionState == ProvisionState::Failed) {
    return;
  }
}

void handleSetWiFi(WebServer& server, Preferences& preferences){
  if (!isTrustedMutationRequest(server)) return;
  if(server.hasArg("ssid") && server.hasArg("password")){
    savedSSID = server.arg("ssid");
    savedPassword = server.arg("password");
    preferences.putString("wifiSSID", savedSSID);
    preferences.putString("wifiPassword", savedPassword);
    server.send(200,"text/plain","ok");
    // Даємо час відправити відповідь клієнту
    delay(500);
    // Перезапускаємо підключення до WiFi
    if(!connectToWiFi()) {
      startAPMode();
    } else {
      // Якщо підключилися успішно, вимикаємо AP mode
      if(isAPMode) {
        WiFi.softAPdisconnect(true);
        isAPMode = false;
        restartStationServices();
      }
    }
  } else {
    server.send(400,"text/plain","Missing ssid or password");
  }
}

void handleForgetWiFi(WebServer& server, Preferences& preferences){
  if (!isTrustedMutationRequest(server)) return;
  preferences.remove("wifiSSID");
  preferences.remove("wifiPassword");
  savedSSID = "";
  savedPassword = "";

  server.send(200,"text/plain","ok");
  delay(200);

  WiFi.disconnect(true, true);
  startAPMode();
}

void handleReconnectWiFi(WebServer& server){
  if (!isTrustedMutationRequest(server)) return;
  server.send(200,"text/plain","ok");
  delay(500);
  // Перезапускаємо підключення
  if(!connectToWiFi()) {
    startAPMode();
  } else {
    if(isAPMode) {
      WiFi.softAPdisconnect(true);
      isAPMode = false;
      restartStationServices();
    }
  }
}

