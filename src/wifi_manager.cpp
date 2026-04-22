#include "wifi_manager.h"
#include <Arduino.h>

// === WiFi Variables ===
String savedSSID = "";
String savedPassword = "";
String apSSID = "FishFeeder-Setup";
String apPassword = "";
bool isAPMode = false;

namespace {
const char* kMutationClientHeader = "X-AquaFeed-Client";
const char* kWebUiClient = "webui";
const char* kBackendClient = "backend";

String buildApSsid() {
  char buffer[24];
  unsigned long chipSuffix = static_cast<unsigned long>(ESP.getEfuseMac() & 0xFFFF);
  snprintf(buffer, sizeof(buffer), "FishFeeder-%04lX", chipSuffix);
  return String(buffer);
}

String buildApPassword() {
  char buffer[24];
  unsigned long long chipId = static_cast<unsigned long long>(ESP.getEfuseMac());
  unsigned long suffix = static_cast<unsigned long>((chipId >> 8) & 0xFFFFFF);
  snprintf(buffer, sizeof(buffer), "AquaFeed-%06lX", suffix);
  return String(buffer);
}

void ensureApCredentials(Preferences& preferences) {
  apSSID = buildApSsid();
  apPassword = preferences.getString("apPassword", "");
  if (apPassword.length() < 12) {
    apPassword = buildApPassword();
    preferences.putString("apPassword", apPassword);
  }
}

void restartStationServices() {
  if (!MDNS.begin("fish")) Serial.println("Error setting up MDNS!");
  else Serial.println("mDNS responder started: http://fish.local");
  configTime(0, 0, "pool.ntp.org", "time.google.com");
}
}  // namespace

void configureRequestSecurity(WebServer& server) {
  const char* headerKeys[] = {kMutationClientHeader};
  server.collectHeaders(headerKeys, 1);
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
    return true;
  }
  server.send(403, "text/plain", "untrusted client");
  return false;
}

// === WiFi Management Functions ===
bool connectToWiFi() {
  if(savedSSID.length() == 0) return false;
  
  // ============================================================================
  // ОПТИМІЗАЦІЯ ШВИДКОСТІ WiFi
  // ============================================================================
  WiFi.mode(WIFI_STA);
  
  // Оптимізації для швидкого підключення
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false); // Вимкнути power save для швидкості
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Максимальна потужність для швидкості
  
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  Serial.print("Connecting to WiFi: " + savedSSID);
  
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
    isAPMode = false;
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi");
    return false;
  }
}

void startAPMode() {
  Serial.println("Starting Access Point mode...");
  WiFi.mode(WIFI_AP);
  
  // Оптимізації для швидкої роботи AP
  WiFi.setSleep(false); // Вимкнути power save для швидкості
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Максимальна потужність
  
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP Mode started");
  Serial.println("SSID: " + apSSID);
  Serial.println("Password: " + apPassword);
  Serial.println("AP IP: " + IP.toString());
  isAPMode = true;
}

void initWiFi(Preferences& preferences) {
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
  server.on("/api/setWiFi", HTTP_POST, [&server, &preferences](){ handleSetWiFi(server, preferences); });
  server.on("/api/forgetWiFi", HTTP_POST, [&server, &preferences](){ handleForgetWiFi(server, preferences); });
  server.on("/api/reconnectWiFi", HTTP_POST, [&server](){ handleReconnectWiFi(server); });
}

// WiFi page definition moved to src/web/wifi_page.cpp.

// === WiFi Handlers ===
void handleWiFi(WebServer& server) {
  server.send(200,"text/html",pageWiFi);
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

