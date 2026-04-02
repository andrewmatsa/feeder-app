#include "wifi_manager.h"

// === WiFi Variables ===
String savedSSID = "";
String savedPassword = "";
const char* apSSID = "FishFeeder-Setup";
const char* apPassword = "12345678";
bool isAPMode = false;

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
  
  WiFi.softAP(apSSID, apPassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP Mode started");
  Serial.println("SSID: " + String(apSSID));
  Serial.println("Password: " + String(apPassword));
  Serial.println("AP IP: " + IP.toString());
  isAPMode = true;
}

void initWiFi(Preferences& preferences) {
  // Завантажуємо збережені WiFi дані
  savedSSID = preferences.getString("wifiSSID", "");
  savedPassword = preferences.getString("wifiPassword", "");
  
  // Якщо немає збережених даних, використовуємо старі значення для сумісності
  if(savedSSID.length() == 0) {
    savedSSID = "Andre Archer Connect";
    savedPassword = "1234567890abb";
  }
  
  // Підключаємося до WiFi
  if(!connectToWiFi()) {
    // Якщо не вдалося підключитися, увімкнути AP mode
    startAPMode();
  }
  
  if(!isAPMode) {
    if(!MDNS.begin("fish")) Serial.println("Error setting up MDNS!");
    else Serial.println("mDNS responder started: http://fish.local");
    configTime(0,0,"pool.ntp.org","time.google.com");
  }
}

void setupWiFiHandlers(WebServer& server, Preferences& preferences) {
  server.on("/wifi", [&server](){ handleWiFi(server); });
  server.on("/api/setWiFi", [&server, &preferences](){ handleSetWiFi(server, preferences); });
  server.on("/api/forgetWiFi", [&server, &preferences](){ handleForgetWiFi(server, preferences); });
  server.on("/api/reconnectWiFi", [&server](){ handleReconnectWiFi(server); });
}

// === WiFi HTML Page ===
const char* pageWiFi = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Налаштування WiFi</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  padding: 8px;
  max-width: 100%;
  margin: auto;
  background: #F5F5F5;
  min-height: 100vh;
  color: #333;
  font-size: 14px;
}
.row {margin-bottom: 12px;}
label {display: block; font-weight: 600; margin-bottom: 4px; color: #2c3e50; font-size: 13px;}
input[type=text], input[type=password] {
  width: 100%;
  padding: 10px;
  border-radius: 10px;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 13px;
  transition: border-color 0.2s ease, box-shadow 0.2s ease;
}
input[type=text]:focus, input[type=password]:focus {
  outline: none;
  border-color: #1976D2;
  box-shadow: 0 0 0 3px rgba(25, 118, 210, 0.15);
}
input[type=number] {
  width: 100%;
  padding: 10px;
  border-radius: 10px;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 13px;
  transition: border-color 0.2s ease, box-shadow 0.2s ease;
  box-sizing: border-box;
}
input[type=number]:focus {
  outline: none;
  border-color: #1976D2;
  box-shadow: 0 0 0 3px rgba(25, 118, 210, 0.15);
}
#deepSleepIdleSec,
#displayOffSec {
  width: 100px;
  max-width: 100%;
  margin-top: 6px;
}
button {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 12px 24px;
  font-size: 13px;
  margin-top: 4px;
  width: 100%;
  border: none;
  border-radius: 999px;
  background: #111;
  color: #fff;
  font-weight: 600;
  letter-spacing: 0.2px;
  cursor: pointer;
  transition: transform 0.15s ease, background 0.2s ease;
}
button:hover {
  transform: translateY(-1px);
  box-shadow: 0 14px 22px rgba(0,0,0,0.2);
}
button:active {
  transform: translateY(0);
  box-shadow: 0 6px 14px rgba(0,0,0,0.16);
}
.card {
  padding: 16px;
  border-radius: 12px;
  background: #FFFFFF;
  margin-bottom: 12px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}
h2 {
  text-align: center;
  color: #333;
  margin-bottom: 12px;
  font-size: 1.4em;
  font-weight: 600;
}
.toast {
  position: fixed;
  top: 20px;
  left: 50%;
  transform: translate(-50%, -10px);
  transform-origin: center;
  background: #111;
  color: #fff;
  padding: 14px 26px;
  border-radius: 999px;
  box-shadow: 0 18px 34px rgba(0, 0, 0, 0.22);
  font-weight: 600;
  font-size: 14px;
  letter-spacing: 0.3px;
  z-index: 1000;
  opacity: 0;
  transition: opacity 0.25s ease, transform 0.25s ease;
  pointer-events: none;
}
.toast.show {
  opacity: 1;
  transform: translate(-50%, 0);
}
.section-header {
  display: flex;
  align-items: flex-start;
  gap: 10px;
  margin-bottom: 16px;
}
.section-icon {
  width: 36px;
  height: 36px;
  border-radius: 12px;
  background: #EEF1F6;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #4A5568;
}
.section-icon svg {
  width: 20px;
  height: 20px;
  stroke: currentColor;
  stroke-width: 1.8;
  stroke-linecap: round;
  stroke-linejoin: round;
  fill: none;
}
.section-icon svg .filled {
  fill: currentColor;
  stroke: none;
}
.section-title {
  font-size: 16px;
  font-weight: 600;
  color: #222;
}
.section-subtitle {
  font-size: 12px;
  color: #9099A6;
  margin-top: 2px;
}
.hero-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin-bottom: 18px;
}
.app-illustration svg {
  width: 65px;
  height: auto;
  display: block;
  transform: translateY(-6px);
  filter: drop-shadow(0 16px 28px rgba(17, 24, 39, 0.2));
}
.hero-bubble {
  animation: hero-bubble-rise 2.4s ease-in-out infinite;
  transform-box: fill-box;
}
.hero-bubble-1 { animation-delay: 0s; }
.hero-bubble-2 { animation-delay: 0.4s; }
.hero-bubble-3 { animation-delay: 0.8s; }
@keyframes hero-bubble-rise {
  0% { transform: translateY(0px); opacity: 0.9; }
  50% { transform: translateY(-6px); opacity: 0.55; }
  100% { transform: translateY(0px); opacity: 0.9; }
}
.hero-svg-fish {
  animation: hero-fish-bob 4.5s ease-in-out infinite;
  transform-origin: 32px 42px;
  transform-box: fill-box;
}
@keyframes hero-fish-bob {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-3.5px); }
}
.app-heading {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 2px;
}
.app-title {
  font-size: 24px;
  font-weight: 700;
  color: #1f2937;
  letter-spacing: 0.4px;
}
.app-subtitle {
  font-size: 13px;
  color: #6b7280;
  letter-spacing: 0.3px;
}
.lang-section-row {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 10px;
  margin-top: 4px;
}
.lang-btn {
  border: 1px solid rgba(0,0,0,0.15);
  background: #fff;
  color: #111;
  border-radius: 999px;
  padding: 5px 10px;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
}
.lang-btn.active {
  background: #111;
  color: #fff;
}
.status-pill.success {
  background: rgba(76,175,80,0.14);
  color: #2e7d32;
}
.status-pill.warning {
  background: rgba(255,152,0,0.14);
  color: #f57c00;
}
.status-pill.error {
  background: rgba(244,67,54,0.14);
  color: #c62828;
}
.status-pill {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  border-radius: 999px;
  background: rgba(0,0,0,0.06);
  font-size: 12px;
  color: #555;
  font-weight: 500;
}
.button-row {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin: 12px 0;
}
.button-row button {
  flex: 1 1 160px;
  min-width: 160px;
  width: auto;
  display: inline-flex;
}
.button-row button.secondary {
  background: #f3f4f6;
  color: #1f2937;
  box-shadow: none;
}
.button-row.ap-mode {
  justify-content: center;
}
.button-row.ap-mode button.secondary {
  display: none;
}
.button-row button.secondary:hover {
  background: #e5e7eb;
}
.button-row button.secondary:active {
  background: #d1d5db;
}
.note-text {
  font-size: 12px;
  color: #666;
  margin-top: 12px;
  line-height: 1.5;
}
.power-toggle {
  display: inline-flex;
  align-items: center;
  gap: 12px;
  cursor: pointer;
  user-select: none;
  margin: 12px 0;
}
.power-toggle input {
  position: absolute;
  opacity: 0;
  pointer-events: none;
}
.power-toggle-box {
  width: 26px;
  height: 26px;
  border-radius: 8px;
  background: #e6e9ef;
  border: 1px solid #d5d9e0;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: background 0.2s ease, border 0.2s ease, transform 0.2s ease;
}
.power-toggle-box::after {
  content: '\2713';
  font-size: 16px;
  color: #fff;
  opacity: 0;
  transition: opacity 0.2s ease;
}
.power-toggle input:checked + .power-toggle-box {
  background: linear-gradient(135deg, #40a4ff, #1c7dff);
  border-color: transparent;
  box-shadow: 0 6px 12px rgba(28, 125, 255, 0.35);
}
.power-toggle input:checked + .power-toggle-box::after {
  opacity: 1;
}
.power-toggle-text {
  font-size: 13px;
  color: #333;
  font-weight: 500;
}
.power-toggle:hover .power-toggle-box {
  border-color: #b8c2d1;
}
.bottom-tabs {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  display: flex;
  background: rgba(255,255,255,0.96);
  box-shadow: 0 -6px 18px rgba(0, 0, 0, 0.15);
  z-index: 1000;
  border-top: 1px solid rgba(15, 23, 42, 0.08);
  padding: 10px 12px;
  border-radius: 16px 16px 0 0;
  backdrop-filter: blur(10px);
}
.bottom-tab {
  flex: 1;
  padding: 10px 8px;
  text-align: center;
  text-decoration: none;
  color: #4b5563;
  font-size: 12px;
  font-weight: 500;
  transition: all 0.2s ease;
  border: none;
  background: transparent;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
  border-radius: 12px;
  margin: 0 6px;
}
.bottom-tab:hover {
  background: rgba(15, 23, 42, 0.06);
}
.bottom-tab-icon {
  font-size: 22px;
  line-height: 1;
  color: inherit;
  display: inline-block;
}
.bottom-tab-icon-svg {
  width: 22px;
  height: 22px;
  stroke: currentColor;
  fill: none;
  stroke-width: 1.9;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.bottom-tab-icon-svg .filled {
  fill: currentColor;
  stroke: none;
}
.bottom-tab.active {
  color: #111827;
  background: rgba(15, 23, 42, 0.1);
}
.bottom-tab.active .bottom-tab-icon {
  color: #111827;
}
.bottom-tab.active .home-icon {
  color: #111827;
}
body {
  padding-bottom: 75px;
}
.page-title {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  margin: 18px 0 20px;
  font-size: 1.4em;
  font-weight: 600;
  color: #333;
}
.page-title svg {
  width: 28px;
  height: 28px;
  stroke: #4A5568;
  stroke-width: 1.8;
  fill: none;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.page-title svg .filled {
  fill: #4A5568;
  stroke: none;
}
</style>
</head>
<body>
<div id="toast" class="toast">Збережено</div>
<div class="hero-header">
  <div class="app-illustration">
    <svg class="hero-svg-fish" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="Стилізована рибка">
      <path d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4" fill="#728389"/>
      <g fill="#8d9ba3">
        <path d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"/>
        <ellipse cx="33.4" cy="35.3" rx="2.2" ry="3.2"/>
        <ellipse cx="37.6" cy="39.2" rx="1.2" ry="2.5"/>
        <ellipse cx="39.9" cy="36" rx=".6" ry="1.7"/>
      </g>
      <g fill="#75d6ff">
        <ellipse class="hero-bubble hero-bubble-1" cx="5.3" cy="44" rx="1.7" ry="1.8"/>
        <ellipse class="hero-bubble hero-bubble-2" cx="6.3" cy="23.4" rx="4.3" ry="4.5"/>
        <ellipse class="hero-bubble hero-bubble-3" cx="12.8" cy="10.3" rx="8" ry="8.3"/>
      </g>
      <ellipse cx="18.7" cy="38.5" rx="7.1" ry="7.4" fill="#fcfcfa"/>
      <ellipse cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c"/>
    </svg>
  </div>
  <div class="app-heading">
    <div class="app-title">AquaFeed Control</div>
    <div class="app-subtitle">Налаштування WiFi</div>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="10"></circle>
        <path d="M2 12h20"></path>
        <path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"></path>
      </svg>
    </div>
    <div>
      <div class="section-title">Мова інтерфейсу</div>
      <div class="section-subtitle"></div>
    </div>
  </div>
  <div class="lang-section-row">
    <button type="button" id="wifiLangUkBtn" class="lang-btn">UK</button>
    <button type="button" id="wifiLangEnBtn" class="lang-btn">EN</button>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M4 9c4.5-4.5 11.5-4.5 16 0"></path>
        <path d="M7 12c2.8-2.8 7.2-2.8 10 0"></path>
        <path d="M10.5 15.5c1-1 3-1 4 0"></path>
        <circle class="filled" cx="12" cy="19" r="1.2"></circle>
      </svg>
    </div>
    <div>
      <div class="section-title">Статус підключення</div>
      <div class="section-subtitle">Поточний режим роботи WiFi</div>
    </div>
  </div>
  <div class="status-pill" id="wifiStatusPill">
    <span id="wifiStatusText">завантаження...</span>
  </div>
  <div class="button-row">
    <button type="button" id="btnWifiReconnect" onclick="reconnectWiFi()" style="background: linear-gradient(45deg, #FF9800, #F57C00);">Перезапустити підключення</button>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 17a2 2 0 1 0 0-4"></path>
        <path d="M5 10V8a7 7 0 0 1 14 0v2"></path>
        <rect x="5" y="10" width="14" height="10" rx="2"></rect>
      </svg>
    </div>
    <div>
      <div class="section-title">Налаштування мережі</div>
      <div class="section-subtitle">Введіть SSID та пароль для підключення</div>
    </div>
  </div>
  <div class="row">
    <label id="wifiLblSsid">SSID (назва мережі):</label>
    <input type="text" id="wifiSSID" placeholder="Введіть назву WiFi або виберіть зі списку" style="width: 100%;">
  </div>
  <div class="row">
    <label id="wifiLblPass">Пароль:</label>
    <input type="password" id="wifiPassword" placeholder="Введіть пароль" style="width: 100%;">
  </div>
  <div class="button-row" id="wifiActions">
    <button type="button" id="btnWifiSave" onclick="saveWiFi()">Зберегти WiFi</button>
    <button type="button" class="secondary" id="btnWifiForget" onclick="forgetWiFi()">Забути мережу</button>
  </div>
  <div class="note-text" id="wifiNoteNetwork">
    <strong>Примітка:</strong> Після збереження пристрій перезапустить підключення. Якщо підключення не вдасться, пристрій створить точку доступу "FishFeeder-Setup" з паролем "12345678". Кнопка «Забути» видаляє збережені креденшіали та одразу повертає пристрій у режим точки доступу, який доступний за адресами <code>http://192.168.4.1</code> або <code>http://fish.local</code>.
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 4.5v7.5"></path>
        <path d="M7.5 7.5a6.5 6.5 0 1 0 9 0"></path>
      </svg>
    </div>
    <div>
      <div class="section-title">Налаштування енергії</div>
      <div class="section-subtitle">Оптимізуйте споживання живлення</div>
    </div>
  </div>
  <label class="power-toggle">
    <input type="checkbox" id="powerSaveMode">
    <span class="power-toggle-box"></span>
    <span class="power-toggle-text" id="wifiLblPowerEco">Режим економії енергії</span>
  </label>
  <div class="note-text" id="wifiNotePower"></div>
  <div class="row" id="deepSleepIdleRow" style="margin-top: 12px;">
    <label for="deepSleepIdleSec" id="labelDeepSleepIdle">Секунд бездіяльності до глибокого сну (10–3600):</label>
    <input type="number" id="deepSleepIdleSec" min="10" max="3600" value="60">
  </div>
  <div class="note-text" id="wifiNotePowerOff" style="margin-top: 12px;"></div>
  <div class="button-row" style="margin-top: 8px;">
    <button type="button" onclick="savePowerMode()" id="btnSavePowerEco">Зберегти режим енергії</button>
  </div>
  <label class="power-toggle" style="margin-top: 16px;">
    <input type="checkbox" id="displayEnabled">
    <span class="power-toggle-box"></span>
    <span class="power-toggle-text" id="wifiLblDisplayToggle">Увімкнути OLED дисплей</span>
  </label>
  <div class="note-text" id="wifiNoteOled">Вимкніть дисплей для економії енергії. Всі функції працюють через веб-інтерфейс.</div>
  <div class="row" id="displayOffRow" style="margin-top: 12px;">
    <label for="displayOffSec" id="lblDisplayOffSec">Секунд до вимкнення OLED (режим економії, 5–600):</label>
    <input type="number" id="displayOffSec" min="5" max="600" value="20">
  </div>
  <div class="note-text" id="wifiNoteDisplayOff" style="font-size: 12px; color: #6b7280;"></div>
  <div class="button-row">
    <button type="button" id="btnWifiSaveDisplay" onclick="saveDisplayMode()">Зберегти дисплей</button>
  </div>
</div>

<script>
let wifiLang = localStorage.getItem('aqua_lang') || 'uk';
function wifiIsEn() { return wifiLang === 'en'; }
function applyWiFiLanguage() {
  document.title = wifiIsEn() ? 'WiFi settings' : 'Налаштування WiFi';
  const toastEl = document.getElementById('toast');
  if (toastEl) toastEl.textContent = wifiIsEn() ? 'Saved' : 'Збережено';
  const lblSsid = document.getElementById('wifiLblSsid');
  if (lblSsid) lblSsid.textContent = wifiIsEn() ? 'SSID (network name):' : 'SSID (назва мережі):';
  const lblPass = document.getElementById('wifiLblPass');
  if (lblPass) lblPass.textContent = wifiIsEn() ? 'Password:' : 'Пароль:';
  const inpSsid = document.getElementById('wifiSSID');
  if (inpSsid) inpSsid.placeholder = wifiIsEn() ? 'Enter WiFi name or pick from list' : 'Введіть назву WiFi або виберіть зі списку';
  const inpPass = document.getElementById('wifiPassword');
  if (inpPass) inpPass.placeholder = wifiIsEn() ? 'Enter password' : 'Введіть пароль';
  const btnRec = document.getElementById('btnWifiReconnect');
  if (btnRec) btnRec.textContent = wifiIsEn() ? 'Restart connection' : 'Перезапустити підключення';
  const btnSave = document.getElementById('btnWifiSave');
  if (btnSave) btnSave.textContent = wifiIsEn() ? 'Save WiFi' : 'Зберегти WiFi';
  const btnForget = document.getElementById('btnWifiForget');
  if (btnForget) btnForget.textContent = wifiIsEn() ? 'Forget network' : 'Забути мережу';
  const noteNet = document.getElementById('wifiNoteNetwork');
  if (noteNet) {
    noteNet.innerHTML = wifiIsEn()
      ? '<strong>Note:</strong> After saving, the device reconnects. If it fails, it opens an access point &quot;FishFeeder-Setup&quot; with password &quot;12345678&quot;. <strong>Forget</strong> clears saved credentials and returns AP mode at <code>http://192.168.4.1</code> or <code>http://fish.local</code>.'
      : '<strong>Примітка:</strong> Після збереження пристрій перезапустить підключення. Якщо підключення не вдасться, пристрій створить точку доступу &quot;FishFeeder-Setup&quot; з паролем &quot;12345678&quot;. Кнопка «Забути» видаляє збережені креденшіали та одразу повертає пристрій у режим точки доступу, який доступний за адресами <code>http://192.168.4.1</code> або <code>http://fish.local</code>.';
  }
  const lblPowerEco = document.getElementById('wifiLblPowerEco');
  if (lblPowerEco) lblPowerEco.textContent = wifiIsEn() ? 'Power saving mode' : 'Режим економії енергії';
  const lblDisp = document.getElementById('wifiLblDisplayToggle');
  if (lblDisp) lblDisp.textContent = wifiIsEn() ? 'Enable OLED display' : 'Увімкнути OLED дисплей';
  const noteOled = document.getElementById('wifiNoteOled');
  if (noteOled) noteOled.textContent = wifiIsEn()
    ? 'Turn off the display to save power. All features remain available in the web interface.'
    : 'Вимкніть дисплей для економії енергії. Всі функції працюють через веб-інтерфейс.';
  const btnDisp = document.getElementById('btnWifiSaveDisplay');
  if (btnDisp) btnDisp.textContent = wifiIsEn() ? 'Save display' : 'Зберегти дисплей';
  const lblDispOff = document.getElementById('lblDisplayOffSec');
  if (lblDispOff) lblDispOff.textContent = wifiIsEn()
    ? 'Seconds until OLED turns off (power save, 5–600):'
    : 'Секунд до вимкнення OLED (режим економії, 5–600):';
  const noteDispOff = document.getElementById('wifiNoteDisplayOff');
  if (noteDispOff) noteDispOff.textContent = wifiIsEn()
    ? 'With power saving on, the display stays on this long after a button press or web use, then blanks. Default 20 s.'
    : 'У режимі економії дисплей лишається увімкненим стільки секунд після кнопки або вебу, потім гасне. За замовчуванням 20 с.';
  const heroFish = document.querySelector('.hero-svg-fish');
  if (heroFish) heroFish.setAttribute('aria-label', wifiIsEn() ? 'Stylized fish' : 'Стилізована рибка');
  const wifiStatusText = document.getElementById('wifiStatusText');
  if (wifiStatusText) {
    const st = wifiStatusText.textContent.trim();
    if (st === 'завантаження...' || st === 'loading...') wifiStatusText.textContent = wifiIsEn() ? 'loading...' : 'завантаження...';
  }
  const tabs = document.querySelectorAll('.bottom-tab span');
  const sectionTitles = document.querySelectorAll('.section-title');
  const sectionSubtitles = document.querySelectorAll('.section-subtitle');
  const appSubtitle = document.querySelector('.app-subtitle');
  if (appSubtitle) appSubtitle.textContent = wifiIsEn() ? 'WiFi settings' : 'Налаштування WiFi';
  if (sectionTitles[0]) sectionTitles[0].textContent = wifiIsEn() ? 'Interface language' : 'Мова інтерфейсу';
  if (sectionSubtitles[0]) sectionSubtitles[0].textContent = '';
  if (sectionTitles[1]) sectionTitles[1].textContent = wifiIsEn() ? 'Connection status' : 'Статус підключення';
  if (sectionSubtitles[1]) sectionSubtitles[1].textContent = wifiIsEn() ? 'Current WiFi mode' : 'Поточний режим роботи WiFi';
  if (sectionTitles[2]) sectionTitles[2].textContent = wifiIsEn() ? 'Network settings' : 'Налаштування мережі';
  if (sectionSubtitles[2]) sectionSubtitles[2].textContent = wifiIsEn() ? 'Enter SSID and password to connect' : 'Введіть SSID та пароль для підключення';
  if (sectionTitles[3]) sectionTitles[3].textContent = wifiIsEn() ? 'Power settings' : 'Налаштування енергії';
  if (sectionSubtitles[3]) sectionSubtitles[3].textContent = wifiIsEn() ? 'Optimize power consumption' : 'Оптимізуйте споживання живлення';
  if (tabs[0]) tabs[0].textContent = wifiIsEn() ? 'Home' : 'Головна';
  if (tabs[1]) tabs[1].textContent = wifiIsEn() ? 'Info' : 'Інформація';
  if (tabs[2]) tabs[2].textContent = wifiIsEn() ? 'Settings' : 'Налаштування';
  const ukBtn = document.getElementById('wifiLangUkBtn');
  const enBtn = document.getElementById('wifiLangEnBtn');
  if (ukBtn) ukBtn.classList.toggle('active', !wifiIsEn());
  if (enBtn) enBtn.classList.toggle('active', wifiIsEn());
  const notePower = document.getElementById('wifiNotePower');
  if (notePower) {
    notePower.innerHTML = wifiIsEn()
      ? '<strong>Power saving on:</strong> after the idle time below, the device enters <strong>deep sleep</strong>. It wakes for the <strong>feed schedule</strong> (timer shortly before the next slot) or when you press the <strong>physical button</strong>. An open web page keeps it awake.'
      : '<strong>Режим економії увімкнено:</strong> після вказаного часу бездіяльності — <strong>глибокий сон</strong>. Пробудження перед <strong>розкладом годування</strong> (таймер) або кнопкою на корпусі. Відкритий веб не дає заснути.';
  }
  const notePowerOff = document.getElementById('wifiNotePowerOff');
  if (notePowerOff) {
    notePowerOff.innerHTML = wifiIsEn()
      ? '<strong>Power saving off:</strong> no deep sleep — device stays active. If the OLED is enabled below, the display stays on.'
      : '<strong>Режим економії вимкнено:</strong> глибокого сну немає — пристрій завжди активний. Якщо нижче увімкнено OLED — дисплей світиться постійно.';
  }
  const labelIdle = document.getElementById('labelDeepSleepIdle');
  if (labelIdle) {
    labelIdle.textContent = wifiIsEn()
      ? 'Idle seconds before deep sleep (10–3600):'
      : 'Секунд бездіяльності до глибокого сну (10–3600):';
  }
  const btnEco = document.getElementById('btnSavePowerEco');
  if (btnEco) {
    btnEco.textContent = wifiIsEn() ? 'Save power mode' : 'Зберегти режим енергії';
  }
  syncPowerEcoUiFromCheckbox();
}
function setWiFiLanguage(lang) {
  wifiLang = lang === 'en' ? 'en' : 'uk';
  localStorage.setItem('aqua_lang', wifiLang);
  applyWiFiLanguage();
  updateStatus();
}
function showToast(text = (wifiIsEn() ? 'Saved' : 'Збережено')) {
  const toast = document.getElementById('toast');
  toast.innerText = text;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2000);
}

function reconnectWiFi(){
  showToast(wifiIsEn() ? 'Restarting connection...' : 'Перезапуск підключення...');
  fetch('/api/reconnectWiFi')
    .then(()=>{
      showToast(wifiIsEn() ? 'Connection restarted' : 'Підключення перезапущено');
      setTimeout(()=>{
        updateStatus();
      }, 2000);
    })
    .catch(()=>{
      showToast(wifiIsEn() ? 'Restart error' : 'Помилка перезапуску');
    });
}

function saveWiFi(){ 
  const ssid = document.getElementById('wifiSSID').value;
  const password = document.getElementById('wifiPassword').value;
  if(!ssid || ssid.trim() === '') {
    showToast(wifiIsEn() ? 'Enter WiFi network name' : 'Введіть назву WiFi мережі');
    return;
  }
  fetch('/api/setWiFi?ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(password))
    .then(()=>{
      showToast(wifiIsEn() ? 'WiFi saved, restarting connection...' : 'WiFi збережено, перезапуск підключення...');
      setTimeout(()=>{
        window.location.reload();
      }, 3000);
    })
    .catch(()=>{
      showToast(wifiIsEn() ? 'WiFi save error' : 'Помилка збереження WiFi');
    });
}

function forgetWiFi(){
  showToast(wifiIsEn() ? 'Removing network...' : 'Видаляю мережу...');
  fetch('/api/forgetWiFi')
    .then(()=>{
      document.getElementById('wifiSSID').value = '';
      document.getElementById('wifiPassword').value = '';
      showToast(wifiIsEn() ? 'Network forgotten. Connect to FishFeeder-Setup' : 'Мережу забуто. Підключіться до FishFeeder-Setup');
      setTimeout(()=>{ window.location.reload(); }, 2000);
    })
    .catch(()=> showToast(wifiIsEn() ? 'WiFi remove error' : 'Помилка видалення WiFi'));
}

function savePowerMode(){
  const enabled = document.getElementById('powerSaveMode').checked;
  const el = document.getElementById('deepSleepIdleSec');
  let idleSec = el ? parseInt(el.value, 10) : 60;
  if (!Number.isFinite(idleSec)) idleSec = 60;
  idleSec = Math.max(10, Math.min(3600, idleSec));
  fetch('/api/setPowerMode?enabled='+(enabled ? 'true' : 'false'))
    .then(() => fetch('/api/setDeepSleep?idleSec='+encodeURIComponent(idleSec)))
    .then(()=>{ showToast(wifiIsEn() ? 'Saved' : 'Збережено'); updateStatus(); })
    .catch(()=> showToast(wifiIsEn() ? 'Save error' : 'Помилка збереження'));
}

function saveDisplayMode(){
  const enabled = document.getElementById('displayEnabled').checked;
  const secEl = document.getElementById('displayOffSec');
  let sec = secEl ? parseInt(secEl.value, 10) : 20;
  if (!Number.isFinite(sec)) sec = 20;
  sec = Math.max(5, Math.min(600, sec));
  if (secEl) secEl.value = sec;
  fetch('/api/setDisplayMode?enabled='+enabled)
    .then(()=> fetch('/api/setDisplayOff?sec='+encodeURIComponent(sec)))
    .then(()=>{ showToast(wifiIsEn() ? 'Saved' : 'Збережено'); updateStatus(); })
    .catch(()=> showToast(wifiIsEn() ? 'Save error' : 'Помилка збереження'));
}

function updateStatus(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    const statusText = document.getElementById('wifiStatusText');
    const statusPill = document.getElementById('wifiStatusPill');
    const actionsRow = document.getElementById('wifiActions');
    if (statusPill) {
      statusPill.classList.remove('success','warning','error');
    }
    statusText.style.color = '';
    const ssidInput = document.getElementById('wifiSSID');
    if(ssidInput) {
      if(j.wifiSSID) {
        ssidInput.value = j.wifiSSID;
      } else {
        ssidInput.value = '';
      }
    }
    const passwordInput = document.getElementById('wifiPassword');
    if (passwordInput && (!j.wifiSSID || j.isAPMode)) {
      passwordInput.value = '';
    }
    if(j.isAPMode) {
      statusText.innerText = (wifiIsEn() ? 'Access Point mode (AP) - ' : 'Режим точки доступу (AP) - ') + (j.wifiSSID || (wifiIsEn() ? 'not set' : 'не налаштовано'));
      if (statusPill) statusPill.classList.add('warning');
      if (actionsRow) actionsRow.style.display = 'flex';
      if (actionsRow) actionsRow.classList.add('ap-mode');
    } else if(j.wifiIP) {
      statusText.innerText = (wifiIsEn() ? 'Connected to: ' : 'Підключено до: ') + (j.wifiSSID || (wifiIsEn() ? 'unknown' : 'невідомо')) + ' (IP: ' + j.wifiIP + ')';
      if (statusPill) statusPill.classList.add('success');
      if (actionsRow) {
        actionsRow.style.display = 'flex';
        actionsRow.classList.remove('ap-mode');
      }
    } else {
      statusText.innerText = wifiIsEn() ? 'Not connected' : 'Не підключено';
      if (statusPill) statusPill.classList.add('error');
      if (actionsRow) actionsRow.style.display = 'flex';
      if (actionsRow) actionsRow.classList.add('ap-mode');
    }
    const powerToggle = document.getElementById('powerSaveMode');
    if (powerToggle) {
      powerToggle.checked = !!j.powerSaveMode;
    }
    const displayToggle = document.getElementById('displayEnabled');
    if (displayToggle) {
      displayToggle.checked = !!j.displayEnabled;
    }
    const dsIdle = document.getElementById('deepSleepIdleSec');
    if (dsIdle && typeof j.deepSleepIdleSec === 'number') {
      dsIdle.value = j.deepSleepIdleSec;
    }
    const displayOffEl = document.getElementById('displayOffSec');
    if (displayOffEl && typeof j.displayOffAfterSec === 'number') {
      displayOffEl.value = j.displayOffAfterSec;
    }
    syncPowerEcoUiFromCheckbox();
  });
}
function syncPowerEcoUiFromCheckbox() {
  const ps = document.getElementById('powerSaveMode');
  const ecoOn = ps && ps.checked;
  const idleRow = document.getElementById('deepSleepIdleRow');
  const dsIdle = document.getElementById('deepSleepIdleSec');
  if (idleRow) idleRow.style.display = ecoOn ? '' : 'none';
  if (dsIdle) dsIdle.disabled = !ecoOn;
  const np = document.getElementById('wifiNotePower');
  const npo = document.getElementById('wifiNotePowerOff');
  if (np) np.style.display = ecoOn ? 'block' : 'none';
  if (npo) npo.style.display = ecoOn ? 'none' : 'block';
}
window.onload=function() {
  const ukBtn = document.getElementById('wifiLangUkBtn');
  const enBtn = document.getElementById('wifiLangEnBtn');
  if (ukBtn) ukBtn.addEventListener('click', () => setWiFiLanguage('uk'));
  if (enBtn) enBtn.addEventListener('click', () => setWiFiLanguage('en'));
  const ps = document.getElementById('powerSaveMode');
  if (ps) ps.addEventListener('change', syncPowerEcoUiFromCheckbox);
  applyWiFiLanguage();
  updateStatus();
};

// Встановлюємо активний таб
document.addEventListener('DOMContentLoaded', function() {
  const currentPath = window.location.pathname;
  const tabs = document.querySelectorAll('.bottom-tab');
  tabs.forEach(tab => {
    if(tab.getAttribute('href') === currentPath) {
      tab.classList.add('active');
    } else {
      tab.classList.remove('active');
    }
  });
});
</script>

<div class="bottom-tabs">
  <a href="/" class="bottom-tab">
    <svg class="bottom-tab-icon home-icon" width="22" height="22" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M10 20V14H14V20H19V12H22L12 3L2 12H5V20H10Z" fill="currentColor"/>
    </svg>
    <span>Головна</span>
  </a>
  <a href="/info" class="bottom-tab">
    <svg class="bottom-tab-icon bottom-tab-icon-svg" viewBox="0 0 24 24">
      <circle cx="12" cy="12" r="10" stroke="currentColor" fill="none" stroke-width="2"/>
      <path d="M12 16v-4" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
      <circle cx="12" cy="8" r="1" fill="currentColor"/>
    </svg>
    <span>Інформація</span>
  </a>
  <a href="/wifi" class="bottom-tab active">
    <svg class="bottom-tab-icon bottom-tab-icon-svg wifi-icon" viewBox="0 0 24 24">
      <path d="M2.5 9.2C7.03 4.66 16.97 4.66 21.5 9.2" />
      <path d="M5.8 12.5C9.12 9.19 14.88 9.19 18.2 12.5" />
      <path d="M9.4 15.9C11.15 14.15 12.85 14.15 14.6 15.9" />
      <circle class="filled" cx="12" cy="19.2" r="1.2" />
    </svg>
    <span>Налаштування</span>
  </a>
</div>
</body>
</html>
)rawliteral";

// === WiFi Handlers ===
void handleWiFi(WebServer& server) {
  server.send(200,"text/html",pageWiFi);
}

void handleSetWiFi(WebServer& server, Preferences& preferences){
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
        // Запускаємо MDNS
        if(!MDNS.begin("fish")) Serial.println("Error setting up MDNS!");
        else Serial.println("mDNS responder started: http://fish.local");
        configTime(0,0,"pool.ntp.org","time.google.com");
      }
    }
  } else {
    server.send(400,"text/plain","Missing ssid or password");
  }
}

void handleForgetWiFi(WebServer& server, Preferences& preferences){
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
  server.send(200,"text/plain","ok");
  delay(500);
  // Перезапускаємо підключення
  if(!connectToWiFi()) {
    startAPMode();
  } else {
    if(isAPMode) {
      WiFi.softAPdisconnect(true);
      isAPMode = false;
      if(!MDNS.begin("fish")) Serial.println("Error setting up MDNS!");
      else Serial.println("mDNS responder started: http://fish.local");
      configTime(0,0,"pool.ntp.org","time.google.com");
    }
  }
}

