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
  
  WiFi.mode(WIFI_STA);
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
  server.on("/api/scanWiFi", [&server](){ handleScanWiFi(server); });
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
  transition: transform 0.15s ease, box-shadow 0.15s ease, background 0.2s ease;
  box-shadow: 0 10px 18px rgba(0,0,0,0.18);
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
  transform: translateX(-50%);
  background: linear-gradient(45deg, #4CAF50, #45a049);
  color: white;
  padding: 12px 24px;
  border-radius: 8px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
  font-weight: 600;
  z-index: 1000;
  opacity: 0;
  transition: opacity 0.3s ease;
  pointer-events: none;
}
.toast.show {
  opacity: 1;
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
.networks-wrapper {
  margin-top: 12px;
  max-height: 220px;
  overflow-y: auto;
}
.networks-title {
  font-size: 12px;
  font-weight: 600;
  color: #555;
  margin-bottom: 6px;
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
.network-item {
  padding: 10px;
  margin-bottom: 6px;
  background: rgba(0,0,0,0.03);
  border-radius: 8px;
  cursor: pointer;
  display: flex;
  justify-content: space-between;
  align-items: center;
  transition: background 0.2s ease;
}
.network-item:hover {
  background: rgba(0,0,0,0.06);
}
.network-signal {
  font-size: 11px;
  color: #666;
  margin-left: 8px;
}
.network-lock {
  font-size: 11px;
  color: #f44336;
  margin-left: 6px;
}
.network-action {
  font-size: 12px;
  color: #2196F3;
}
.networks-empty {
  padding: 10px;
  border-radius: 8px;
  background: rgba(0,0,0,0.03);
  color: #666;
  font-size: 12px;
  text-align: center;
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
  background: #ffffff;
  box-shadow: 0 -10px 30px rgba(15, 23, 42, 0.08);
  z-index: 1000;
  border-top: 1px solid rgba(15, 23, 42, 0.05);
  padding: 12px 6px;
  border-radius: 20px 20px 0 0;
}
.bottom-tab {
  flex: 1;
  padding: 10px 6px;
  text-align: center;
  text-decoration: none;
  color: #a6adb8;
  font-size: 12px;
  font-weight: 500;
  transition: background 0.2s ease, color 0.2s ease, transform 0.2s ease;
  border: none;
  background: transparent;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
  border-radius: 14px;
}
.bottom-tab:hover {
  background: rgba(166, 173, 184, 0.12);
}
.bottom-tab-icon {
  font-size: 22px;
  line-height: 1;
  color: inherit;
  display: inline-flex;
}
.bottom-tab-icon-svg {
  width: 22px;
  height: 22px;
  stroke: currentColor;
  fill: none;
  stroke-width: 1.8;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.bottom-tab-icon-svg .filled {
  fill: currentColor;
  stroke: none;
}
.bottom-tab.active {
  color: #323845;
  background: rgba(50, 56, 69, 0.12);
}
.bottom-tab.active .bottom-tab-icon {
  color: #1976D2;
}
.bottom-tab.active .home-icon {
  color: #666;
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
<div class="page-title">
  <svg viewBox="0 0 24 24">
    <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09a1.65 1.65 0 0 0-1-1.51 1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09a1.65 1.65 0 0 0 1.51-1 1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
    <circle class="filled" cx="12" cy="12" r="3" />
  </svg>
  <span>Налаштування WiFi</span>
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
    <button onclick="reconnectWiFi()" style="background: linear-gradient(45deg, #FF9800, #F57C00);">Перезапустити підключення</button>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 3v3"></path>
        <path d="M12 18v3"></path>
        <path d="M3 12h3"></path>
        <path d="M18 12h3"></path>
        <path d="M5.64 5.64l2.12 2.12"></path>
        <path d="M16.24 16.24l2.12 2.12"></path>
        <circle cx="12" cy="12" r="3"></circle>
      </svg>
    </div>
    <div>
      <div class="section-title">Пошук мереж</div>
      <div class="section-subtitle">Скануйте та обирайте точку доступу</div>
    </div>
  </div>
  <div class="button-row">
    <button onclick="scanWiFi()" style="background: linear-gradient(45deg, #2196F3, #1976D2);">Сканувати мережі</button>
  </div>
  <div id="wifiList" class="networks-wrapper" style="display: none;">
    <div class="networks-title">Доступні мережі</div>
    <div id="wifiNetworks"></div>
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
    <label>SSID (назва мережі):</label>
    <input type="text" id="wifiSSID" placeholder="Введіть назву WiFi або виберіть зі списку" style="width: 100%;">
  </div>
  <div class="row">
    <label>Пароль:</label>
    <input type="password" id="wifiPassword" placeholder="Введіть пароль" style="width: 100%;">
  </div>
  <div class="button-row">
    <button onclick="saveWiFi()">Зберегти WiFi</button>
  </div>
  <div class="note-text">
    <strong>Примітка:</strong> Після збереження пристрій перезапустить підключення. Якщо підключення не вдасться, пристрій створить точку доступу "FishFeeder-Setup" з паролем "12345678".
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
    <span class="power-toggle-text">Режим економії енергії</span>
  </label>
  <div class="button-row">
    <button onclick="savePowerMode()">Зберегти енергозбереження</button>
  </div>
</div>

<script>
function showToast(text = 'Збережено') {
  const toast = document.getElementById('toast');
  toast.innerText = text;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2000);
}

function scanWiFi(){
  showToast('Сканування мереж...');
  fetch('/api/scanWiFi')
    .then(r=>r.json())
    .then(networks=>{
      const listDiv = document.getElementById('wifiList');
      const networksDiv = document.getElementById('wifiNetworks');
      networksDiv.innerHTML = '';
      
      if(networks.length === 0) {
        networksDiv.innerHTML = '<div class="networks-empty">Мережі не знайдено</div>';
      } else {
        networks.forEach(net => {
          const netDiv = document.createElement('div');
          netDiv.className = 'network-item';
          netDiv.onclick = function() {
            document.getElementById('wifiSSID').value = net.ssid;
            document.getElementById('wifiPassword').focus();
          };
          netDiv.innerHTML = `
            <div>
              <strong>${net.ssid}</strong>
              <span class="network-signal">${net.rssi} dBm</span>
              ${net.encrypted ? '<span class="network-lock">🔒</span>' : ''}
            </div>
            <span class="network-action">Обрати</span>
          `;
          networksDiv.appendChild(netDiv);
        });
      }
      listDiv.style.display = 'block';
      showToast('Сканування завершено');
    })
    .catch(()=>{
      showToast('Помилка сканування');
    });
}

function reconnectWiFi(){
  showToast('Перезапуск підключення...');
  fetch('/api/reconnectWiFi')
    .then(()=>{
      showToast('Підключення перезапущено');
      setTimeout(()=>{
        updateStatus();
      }, 2000);
    })
    .catch(()=>{
      showToast('Помилка перезапуску');
    });
}

function saveWiFi(){ 
  const ssid = document.getElementById('wifiSSID').value;
  const password = document.getElementById('wifiPassword').value;
  if(!ssid || ssid.trim() === '') {
    showToast('Введіть назву WiFi мережі');
    return;
  }
  fetch('/api/setWiFi?ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(password))
    .then(()=>{
      showToast('WiFi збережено, перезапуск підключення...');
      setTimeout(()=>{
        window.location.reload();
      }, 3000);
    })
    .catch(()=>{
      showToast('Помилка збереження WiFi');
    });
}

function savePowerMode(){
  const enabled = document.getElementById('powerSaveMode').checked;
  fetch('/api/setPowerMode?enabled='+enabled)
    .then(()=>{ showToast('Збережено'); updateStatus(); })
    .catch(()=> showToast('Помилка збереження'));
}

function updateStatus(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    const statusText = document.getElementById('wifiStatusText');
    const statusPill = document.getElementById('wifiStatusPill');
    if (statusPill) {
      statusPill.classList.remove('success','warning','error');
    }
    statusText.style.color = '';
    if(j.wifiSSID) {
      document.getElementById('wifiSSID').value=j.wifiSSID;
    }
    if(j.isAPMode) {
      statusText.innerText = 'Режим точки доступу (AP) - ' + (j.wifiSSID || 'не налаштовано');
      if (statusPill) statusPill.classList.add('warning');
    } else if(j.wifiIP) {
      statusText.innerText = 'Підключено до: ' + (j.wifiSSID || 'невідомо') + ' (IP: ' + j.wifiIP + ')';
      if (statusPill) statusPill.classList.add('success');
    } else {
      statusText.innerText = 'Не підключено';
      if (statusPill) statusPill.classList.add('error');
    }
    const powerToggle = document.getElementById('powerSaveMode');
    if (powerToggle) {
      powerToggle.checked = !!j.powerSaveMode;
    }
  });
}
window.onload=updateStatus;

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

void handleScanWiFi(WebServer& server){
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  
  String json = "[";
  for(int i = 0; i < n; i++) {
    if(i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\""+WiFi.SSID(i)+"\",";
    json += "\"rssi\":"+String(WiFi.RSSI(i))+",";
    json += "\"encrypted\":"+String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]";
  
  server.send(200, "application/json", json);
  Serial.println("WiFi scan completed, found " + String(n) + " networks");
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

