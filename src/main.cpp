#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include "time.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "wifi_manager.h"

Servo mg996r;
Preferences preferences;
WebServer server(80);


// === Hardware pins ===
const int SERVO_PIN = 4;
const int BUTTON_PIN = 3;
const int BATTERY_PIN = 2; // ⚡ MH Electronic Voltage Sensor (VOUT)

// === Servo / settings ===
int minAngle = 0;
int maxAngle = 180;
float speedSetting = 20.0;
int currentAngle = 0;
bool manualMoving = false;
bool lastButtonState = HIGH;

// --- Автоматичне годування ---
struct FeedTime {
  int hour;
  int minute;
  int repeats;
  bool done;
};

#define MAX_FEED_TIMES 20
FeedTime feedTimes[MAX_FEED_TIMES];
int feedTimesCount = 0;

// Старі змінні для сумісності
int feedHour1 = 10;
int feedMinute1 = 0;
int feedHour2 = 20;
int feedMinute2 = 0;
bool feed1Done = false;
bool feed2Done = false;

// --- Кількість повторів годування ---
int feedRepeats = 1;
int feedRepeats1 = 1;  // для першого годування
int feedRepeats2 = 1;  // для другого годування

// --- Режим економії енергії ---
bool powerSaveMode = true;  // режим економії енергії
unsigned long lastActivity = 0;
const unsigned long ACTIVITY_TIMEOUT = 300000; // 5 хвилин бездіяльності
const unsigned long SLEEP_INTERVAL = 60000;    // сон на 1 хвилину між перевірками

// --- Напруга батареї ---
float batteryVoltage = 0.0;
float batteryPercent = 0.0;
// === Оновлено для 2 x 18650 з калібруванням ===
const float DIVIDER_RATIO = 3.3;       // базовий дільник
const float CALIBRATION_FACTOR = 0.886; // підкоригувати під мультиметр

// === Utilities ===
float readBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  // 12-бітний ADC, 3.3V max, множимо на коефіцієнт дільника та калібруємо
  float voltage = (raw / 4095.0) * 3.3 * DIVIDER_RATIO;
  voltage *= CALIBRATION_FACTOR; // точна підгонка під мультиметр
  return voltage;
}

float voltageToPercent(float v) {
  // Орієнтовно для 2S Li-Ion (8.4V max, 6.0V min)
  if (v >= 8.4) return 100.0;
  if (v <= 6.0) return 0.0;
  return (v - 6.0) / (8.4 - 6.0) * 100.0;
}

int speedToStepDelayMs(float sliderSpeed) {
  float minSpeed = 19.5;
  float maxSpeed = 20.0;
  float normalized = (sliderSpeed - 1) / (20 - 1);
  float realSpeed = minSpeed + normalized * (maxSpeed - minSpeed);
  int stepDelay = (int)((20.0 - realSpeed) * 10);
  return max(stepDelay, 0);
}


// === Power Management ===
void enterLightSleep() {
  Serial.println("Перехід в легкий сон для економії енергії...");
  esp_wifi_stop();
  esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL * 1000); // пробудження через 1 хвилину
  esp_light_sleep_start();
  esp_wifi_start();
  if(!isAPMode && savedSSID.length() > 0) {
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    while(WiFi.status() != WL_CONNECTED) { delay(100); }
    Serial.println("Пробудження зі сну, WiFi відновлено");
  } else if(isAPMode) {
    startAPMode();
  }
}

void updateActivity() {
  lastActivity = millis();
}

bool isTimeForFeeding() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  if (!localtime_r(&now, &timeinfo)) return false;
  
  int curHour = timeinfo.tm_hour;
  int curMinute = timeinfo.tm_min;
  
  return (curHour == feedHour1 && curMinute == feedMinute1 && !feed1Done) ||
         (curHour == feedHour2 && curMinute == feedMinute2 && !feed2Done);
}

void moveServoSmooth(int target) {
  target = constrain(target, 0, 180);
  if (target == currentAngle) return;
  int stepDelay = speedToStepDelayMs(speedSetting);
  if (target > currentAngle) {
    for (int a = currentAngle + 1; a <= target; ++a) { mg996r.write(a); delay(stepDelay); }
  } else {
    for (int a = currentAngle - 1; a >= target; --a) { mg996r.write(a); delay(stepDelay); }
  }
  currentAngle = target;
}

void moveServoFast(int target) {
  target = constrain(target, 0, 180);
  mg996r.write(target);
  currentAngle = target;
}

void feedSequence(int repeats = 1) {
  manualMoving = true;
  for (int i = 0; i < repeats; i++) {
    moveServoSmooth(minAngle);
    delay(50);
    moveServoSmooth(maxAngle);
    delay(50);
    moveServoSmooth(minAngle);
    delay(50);
  }
  manualMoving = false;
}

// === Web page ===
const char* pageIndex = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>AquaFeed Control</title>
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
.row {margin-bottom: 8px;}
label {display: block; font-weight: 600; margin-bottom: 4px; color: #2c3e50; font-size: 13px;}
input[type=range], input[type=number], select {width: 95%; padding: 6px; border-radius: 6px; border: 1px solid #ddd; font-size: 13px;}
input[type=checkbox] {transform: scale(1.1); margin-right: 6px;}
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
button.add-btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: #fff;
  color: #333;
  border: 1px solid rgba(15, 23, 42, 0.12);
  box-shadow: none;
  width: auto;
  padding: 10px 22px;
  margin: 8px 0;
  transition: background 0.2s ease, color 0.2s ease;
}
button.add-btn:hover {
  background: rgba(15, 23, 42, 0.08);
  color: #111;
}
button.remove-btn {
  width: 28px;
  height: 28px;
  padding: 0;
  margin-left: auto;
  font-size: 18px;
  line-height: 1;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #f03d3d;
  box-shadow: 0 8px 16px rgba(240,61,61,0.2);
}
button.remove-btn:hover {
  background: #d83232;
  box-shadow: 0 10px 20px rgba(240,61,61,0.24);
}
button.remove-btn:active {
  background: #c62828;
  box-shadow: 0 6px 14px rgba(240,61,61,0.18);
}
.feed-block {
  margin-bottom: 8px;
  padding: 8px;
  background: rgba(0, 0, 0, 0.03);
  border-radius: 6px;
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
.flex-row {
  display: flex;
  gap: 4px;
  align-items: center;
  margin-bottom: 6px;
  flex-wrap: wrap;
}
.flex-row span {font-weight: 500; color: #555; font-size: 12px;}
.flex-row input, .flex-row select {width: auto; min-width: 60px; font-size: 12px;}
.toast {
  position: fixed;
  top: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: #111;
  color: #fff;
  padding: 14px 26px;
  border-radius: 999px;
  box-shadow: 0 18px 34px rgba(0,0,0,0.22);
  font-weight: 600;
  font-size: 14px;
  letter-spacing: 0.3px;
  z-index: 1000;
  opacity: 0;
  transform-origin: center;
  transform: translate(-50%, -10px);
  transition: opacity 0.25s ease, transform 0.25s ease;
  pointer-events: none;
}
.toast.show {
  opacity: 1;
  transform: translate(-50%, 0);
}
.battery-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
  text-align: center;
  padding: 20px;
}
.battery-gauge-wrapper {
  position: relative;
  flex: 0 0 150px;
  max-width: 150px;
  margin: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
}
.battery-title {
  font-size: 14px;
  font-weight: 600;
  color: #222;
}
.battery-subtitle {
  font-size: 12px;
  color: #888;
}
.battery-info-row {
  display: flex;
  justify-content: center;
  align-items: flex-start;
  gap: 20px;
  width: 100%;
}
.battery-gauge-svg {
  width: 100%;
  height: auto;
}
.battery-gauge-percent {
  font-size: 32px;
  font-weight: 700;
  color: #222;
}
.battery-gauge-note {
  font-size: 12px;
  color: #9E9E9E;
}
.next-feed-card {
  flex: 0 0 150px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
  text-align: center;
}
.next-feed-title {
  font-size: 14px;
  font-weight: 600;
  color: #333;
}
.next-feed-label {
  font-size: 12px;
  color: #666;
}
.next-feed-gauge {
  width: 100%;
}
.next-feed-gauge svg {
  width: 100%;
  height: auto;
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
.section-header {
  display: flex;
  align-items: flex-start;
  gap: 10px;
  margin-bottom: 12px;
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
  stroke: #4A5568;
  stroke-width: 1.8;
  fill: none;
  stroke-linecap: round;
  stroke-linejoin: round;
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
.bottom-tabs {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  display: flex;
  background: #f5f5f5;
  box-shadow: 0 -2px 10px rgba(0, 0, 0, 0.1);
  z-index: 1000;
  border-top: 1px solid rgba(0, 0, 0, 0.05);
  padding: 8px 0;
  border-radius: 12px 12px 0 0;
}
.bottom-tab {
  flex: 1;
  padding: 8px 4px;
  text-align: center;
  text-decoration: none;
  color: #666;
  font-size: 11px;
  font-weight: 500;
  transition: all 0.2s ease;
  border: none;
  background: #f5f5f5;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
  border-radius: 8px;
  margin: 0 4px;
}
.bottom-tab.active {
  color: #666;
}
.bottom-tab:hover {
  background: #e8e8e8;
}
.bottom-tab-icon {
  font-size: 22px;
  line-height: 1;
  color: #666;
  display: inline-block;
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
.bottom-tab.active .bottom-tab-icon {
  color: #666;
}
.bottom-tab.active .home-icon {
  color: #666;
}
body {
  padding-bottom: 75px;
}
.hero-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin-bottom: 18px;
}
.app-illustration svg {
  width: 110px;
  height: 46px;
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
</style>
</head>
<body>
<div id="toast" class="toast">Збережено</div>
<div class="hero-header">
  <div class="app-illustration">
    <svg viewBox="0 0 140 60" xmlns="http://www.w3.org/2000/svg">
      <g transform="translate(-5,-5)" opacity="0.9">
        <path fill="#f7931e" d="M25 35c0-10.5 12-19 27-19 9 0 16 8.5 16 19s-7 19-16 19c-15 0-27-8.5-27-19z"/>
        <polygon fill="#f7931e" points="22,35 6,22 6,48"/>
        <circle cx="55" cy="35" r="4.5" fill="#fff"/>
        <circle cx="55" cy="35" r="2.2" fill="#111"/>

        <path fill="#3498db" d="M83 30c0-7 8-13 18-13 6 0 12 6 12 13s-6 13-12 13c-10 0-18-6-18-13z"/>
        <polygon fill="#3498db" points="81,30 72,23 72,37"/>
        <circle cx="103" cy="30" r="3" fill="#fff"/>
        <circle cx="103" cy="30" r="1.5" fill="#111"/>

        <path fill="#8dc63f" d="M118 26c0-4 4-7 9-7 3 0 6 3 6 7s-3 7-6 7c-5 0-9-3-9-7z"/>
        <polygon fill="#8dc63f" points="117,26 111,21 111,31"/>
        <circle cx="132" cy="26" r="1.6" fill="#fff"/>
        <circle cx="132" cy="26" r="0.8" fill="#111"/>
      </g>
    </svg>
  </div>
  <div class="app-heading">
    <div class="app-title">AquaFeed Control</div>
    <div class="app-subtitle">Розумна годівниця</div>
  </div>
</div>

<div class="card battery-card" style="margin-top: 0;">
  <div class="battery-info-row">
    <div class="battery-gauge-wrapper">
      <svg id="batteryGauge" class="battery-gauge-svg" viewBox="0 0 260 160">
        <path id="gaugeBg" d="M 20 140 A 110 110 0 0 1 240 140"
              fill="none" stroke="#E6E9EF" stroke-width="14" stroke-linecap="round"/>
        <path id="gaugeFill" d="M 20 140 A 110 110 0 0 1 240 140"
              fill="none" stroke="#FF5E5E" stroke-width="14" stroke-linecap="round"
              stroke-dasharray="345.58" stroke-dashoffset="345.58" style="transition: stroke-dashoffset 0.6s ease, stroke 0.3s ease;"/>
        <text id="batteryPercent" x="130" y="90" text-anchor="middle" dominant-baseline="middle"
              font-size="34" font-weight="700" fill="#222">--%</text>
      </svg>
      <div class="battery-title">Стан батареї</div>
      <div class="battery-subtitle">Напруга: <span id="batteryVoltage">--</span> В</div>
    </div>
    <div class="next-feed-card">
      <div class="next-feed-gauge">
        <svg id="nextFeedGauge" viewBox="0 0 260 160">
          <path id="nextFeedBg" d="M 20 140 A 110 110 0 0 1 240 140"
                fill="none" stroke="#E6E9EF" stroke-width="14" stroke-linecap="round"/>
          <path id="nextFeedFill" d="M 20 140 A 110 110 0 0 1 240 140"
                fill="none" stroke="#1976D2" stroke-width="14" stroke-linecap="round"
                stroke-dasharray="345.58" stroke-dashoffset="345.58" style="transition: stroke-dashoffset 0.6s ease, stroke 0.3s ease;"/>
          <text id="nextFeedPercent" x="130" y="90" text-anchor="middle" dominant-baseline="middle"
                font-size="30" font-weight="600" fill="#1976D2">— год — хв</text>
        </svg>
      </div>
      <div class="next-feed-title">До наступного годування</div>
    </div>
  </div>
  <div class="battery-gauge-note">Оновлено щойно</div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <line x1="8" y1="5" x2="8" y2="19"></line>
        <line x1="16" y1="5" x2="16" y2="19"></line>
        <circle cx="8" cy="10" r="2.5"></circle>
        <circle cx="16" cy="14" r="2.5"></circle>
      </svg>
    </div>
    <div>
      <div class="section-title">Ручне керування</div>
      <div class="section-subtitle">Керуйте серво-приводом вручну</div>
    </div>
  </div>
  <div class="row">
    <label>Кут серво: <span id="angleLabel">0</span>°</label>
    <input id="angleSlider" type="range" min="0" max="180" value="0">
  </div>
  <div class="row">
    <label>Швидкість серво: <span id="speedValue">20</span></label>
    <input id="speedSlider" type="range" min="1" max="20" step="0.1" value="20" oninput="updateSpeed(this.value)">
  </div>
  <button onclick="saveSpeed()">Зберегти швидкість</button>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M5 11h14"></path>
        <path d="M7 11v2a5 5 0 0 0 10 0v-2"></path>
        <path d="M9 6.5l1.2 3"></path>
        <path d="M15 6.5l-1.2 3"></path>
      </svg>
    </div>
    <div>
      <div class="section-title">Ручне годування</div>
      <div class="section-subtitle">Швидкий запуск циклу годування</div>
    </div>
  </div>
  <div class="flex-row">
    <span>Кількість повторів</span>
    <input id="feedRepeats" type="number" min="1" max="20" value="1">
  </div>
  <button onclick="saveRepeats()" style="margin-top: 0;">Зберегти</button>
  <button onclick="feedNow()" style="margin-top: 12px; background: linear-gradient(45deg, #f44336, #d32f2f);">Годувати зараз</button>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="7"></circle>
        <line x1="12" y1="12" x2="12" y2="7.5"></line>
        <line x1="12" y1="12" x2="15.5" y2="13.5"></line>
        <path d="M4.5 5.5l1.5 1.5"></path>
        <path d="M19.5 5.5l-1.5 1.5"></path>
      </svg>
    </div>
    <div>
      <div class="section-title">Автоматичне годування</div>
      <div class="section-subtitle">Налаштуйте розклад годувань</div>
    </div>
  </div>
  <div id="feedTimesContainer">
    <!-- Блоки будуть додаватися динамічно -->
  </div>
  <button class="add-btn" onclick="addFeedTime()">+ Додати годування</button>
  <button onclick="saveFeedTimes()" style="margin-top: 8px;">Зберегти всі часи</button>
</div>

<script>
function updateAngleLabel(v){ document.getElementById('angleLabel').innerText=v; }
function updateSpeed(v){ document.getElementById('speedValue').innerText=v; }

document.getElementById('angleSlider').addEventListener('input', function(){
  const val = this.value;
  updateAngleLabel(val);
  fetch('/api/setAngle?angle='+val);
});

function showToast(text = 'Збережено') {
  const toast = document.getElementById('toast');
  toast.innerText = text;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2000);
}

function feedNow(){ fetch('/api/feedNow').then(()=>{statusUpdate(); showToast('Годую');}); }
function saveSpeed(){ const s=document.getElementById('speedSlider').value; fetch('/api/setSpeed?speed='+s).then(()=>{statusUpdate(); showToast('Збережено');}); }
function saveRepeats(){ const r=document.getElementById('feedRepeats').value; fetch('/api/setRepeats?repeats='+r).then(()=>{statusUpdate(); showToast();}); }
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
        statusUpdate();
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
let feedTimeCounter = 0;

function addFeedTime(hour = 10, minute = 0, repeats = 1) {
  const container = document.getElementById('feedTimesContainer');
  const blockId = 'feedBlock_' + feedTimeCounter++;
  const block = document.createElement('div');
  block.className = 'feed-block';
  block.id = blockId;
  block.innerHTML = `
    <div class="flex-row">
      <span>Час:</span>
      <input type="number" class="feed-hour" min="0" max="23" value="${hour}" style="width:35px; min-width:35px; padding: 4px;">
      <span>:</span>
      <input type="number" class="feed-minute" min="0" max="59" value="${minute}" style="width:35px; min-width:35px; padding: 4px;">
      <span>Повторів:</span>
      <input type="number" class="feed-repeats" min="1" max="20" value="${repeats}" style="width:35px; min-width:35px; padding: 4px;">
      <button class="remove-btn" onclick="removeFeedTime('${blockId}')" title="Видалити">×</button>
    </div>
  `;
  container.appendChild(block);
}

function removeFeedTime(blockId) {
  const block = document.getElementById(blockId);
  if (block) block.remove();
}

function saveFeedTimes(){
  const blocks = document.querySelectorAll('.feed-block');
  const feedTimes = [];
  blocks.forEach(block => {
    const hour = block.querySelector('.feed-hour').value;
    const minute = block.querySelector('.feed-minute').value;
    const repeats = block.querySelector('.feed-repeats').value;
    feedTimes.push({h: hour, m: minute, r: repeats});
  });
  const data = JSON.stringify(feedTimes);
  fetch('/api/setFeedTimes?data=' + encodeURIComponent(data)).then(()=>{statusUpdate(); showToast();});
}


function loadFeedTimes(feedTimes) {
  const container = document.getElementById('feedTimesContainer');
  container.innerHTML = '';
  if (feedTimes && feedTimes.length > 0) {
    feedTimes.forEach(ft => {
      addFeedTime(ft.h || ft.hour || 10, ft.m || ft.minute || 0, ft.r || ft.repeats || 1);
    });
  } else {
    addFeedTime(10, 0, 1);
  }
}

function normalizeFeedSchedule(status) {
  const schedule = [];
  if (status.feedTimes && status.feedTimes.length) {
    status.feedTimes.forEach(ft => {
      const rawHour = ft.h !== undefined ? ft.h : (ft.hour !== undefined ? ft.hour : 0);
      const rawMinute = ft.m !== undefined ? ft.m : (ft.minute !== undefined ? ft.minute : 0);
      const hour = parseInt(rawHour, 10);
      const minute = parseInt(rawMinute, 10);
      if (!isNaN(hour) && !isNaN(minute)) {
        const normHour = ((hour % 24) + 24) % 24;
        const normMinute = ((minute % 60) + 60) % 60;
        schedule.push({ hour: normHour, minute: normMinute, total: normHour * 60 + normMinute });
      }
    });
  } else if (status.feedHour1 !== undefined && status.feedMinute1 !== undefined) {
    const h1 = Number(status.feedHour1);
    const m1 = Number(status.feedMinute1);
    const h2 = Number(status.feedHour2);
    const m2 = Number(status.feedMinute2);
    if (!isNaN(h1) && !isNaN(m1)) {
      const normHour = ((h1 % 24) + 24) % 24;
      const normMinute = ((m1 % 60) + 60) % 60;
      schedule.push({ hour: normHour, minute: normMinute, total: normHour * 60 + normMinute });
    }
    if (!isNaN(h2) && !isNaN(m2)) {
      const normHour = ((h2 % 24) + 24) % 24;
      const normMinute = ((m2 % 60) + 60) % 60;
      schedule.push({ hour: normHour, minute: normMinute, total: normHour * 60 + normMinute });
    }
  }
  schedule.sort((a, b) => a.total - b.total);
  return schedule;
}

function formatTimeHM(hour, minute) {
  return `${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}`;
}

function formatDurationMinutes(minutes) {
  if (minutes <= 0) return '0 год 0 хв';
  const hours = Math.floor(minutes / 60);
  const mins = minutes % 60;
  return `${hours} год ${mins} хв`;
}

function updateNextFeedingProgress(status) {
  const percentTextEl = document.getElementById('nextFeedPercent');
  const fillPath = document.getElementById('nextFeedFill');
  if (!percentTextEl || !fillPath) return;

  const schedule = normalizeFeedSchedule(status);
  if (!schedule.length) {
    percentTextEl.innerText = '— год — хв';
    fillPath.style.strokeDasharray = 345.58;
    fillPath.style.strokeDashoffset = 345.58;
    return;
  }

  const now = new Date();
  const nowMinutes = now.getHours() * 60 + now.getMinutes();

  let nextIndex = schedule.findIndex(item => item.total > nowMinutes);
  if (nextIndex === -1) nextIndex = 0;
  const next = schedule[nextIndex];
  const prev = schedule[(nextIndex - 1 + schedule.length) % schedule.length];

  let minutesUntilNext = next.total - nowMinutes;
  if (minutesUntilNext <= 0) minutesUntilNext += 24 * 60;

  let interval = next.total - prev.total;
  if (interval <= 0) interval += 24 * 60;

  const elapsed = Math.max(0, interval - minutesUntilNext);
  const percent = Math.max(0, Math.min(100, (elapsed / interval) * 100));

  const circumference = 345.58;
  fillPath.style.strokeDasharray = circumference;
  const offset = circumference - (percent / 100) * circumference;
  fillPath.style.strokeDashoffset = offset;
  percentTextEl.innerText = formatDurationMinutes(minutesUntilNext);
}

function updateBatteryGauge(percent) {
  const gaugeFill = document.getElementById('gaugeFill');
  const percentLabel = document.getElementById('batteryPercent');
  if (!gaugeFill || !percentLabel) return;

  const safePercent = Math.max(0, Math.min(100, percent));
  const radius = 120;
  const circumference = Math.PI * radius;
  const offset = circumference - (safePercent / 100) * circumference;

  gaugeFill.style.strokeDasharray = circumference;
  gaugeFill.style.strokeDashoffset = offset;
  percentLabel.innerText = `${safePercent}%`;

  let color = '#FF5E5E';
  if (safePercent >= 80) {
    color = '#4CAF50';
  } else if (safePercent >= 40) {
    color = '#FF5E5E';
  } else {
    color = '#D32F2F';
  }

  gaugeFill.style.stroke = color;
  percentLabel.style.color = color;
}

function statusUpdate(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    const batteryPercent = Math.round(j.batteryPercent);
    document.getElementById('batteryVoltage').innerText=j.batteryVoltage.toFixed(2);
    updateBatteryGauge(batteryPercent);
    updateNextFeedingProgress(j);
    document.getElementById('angleSlider').value=j.currentAngle; updateAngleLabel(j.currentAngle);
    document.getElementById('speedSlider').value=j.speed; updateSpeed(j.speed);
    document.getElementById('feedRepeats').value=j.feedRepeats;
    if(j.wifiSSID) {
      document.getElementById('wifiSSID').value=j.wifiSSID;
    }
    // Оновлюємо статус WiFi
    const statusText = document.getElementById('wifiStatusText');
    if(j.isAPMode) {
      statusText.innerText = 'Режим точки доступу (AP) - ' + (j.wifiSSID || 'не налаштовано');
      statusText.style.color = '#FF9800';
    } else if(j.wifiIP) {
      statusText.innerText = 'Підключено до: ' + (j.wifiSSID || 'невідомо') + ' (IP: ' + j.wifiIP + ')';
      statusText.style.color = '#4CAF50';
    } else {
      statusText.innerText = 'Не підключено';
      statusText.style.color = '#f44336';
    }
    
    // Завантажуємо динамічні годування
    if (j.feedTimes) {
      loadFeedTimes(j.feedTimes);
    } else if (j.feedHour1 !== undefined) {
      // Сумісність зі старим форматом
      loadFeedTimes([
        {h: j.feedHour1, m: j.feedMinute1, r: j.feedRepeats1 || 1},
        {h: j.feedHour2, m: j.feedMinute2, r: j.feedRepeats2 || 1}
      ]);
    }
  });
}
setInterval(statusUpdate,30000); // зменшено з 5 до 30 секунд
window.onload=function(){
  statusUpdate();
  // Встановлюємо активний таб
  const currentPath = window.location.pathname;
  const tabs = document.querySelectorAll('.bottom-tab');
  tabs.forEach(tab => {
    if(tab.getAttribute('href') === currentPath || (currentPath === '/' && tab.getAttribute('href') === '/')) {
      tab.classList.add('active');
    } else {
      tab.classList.remove('active');
    }
  });
};
</script>

<div class="bottom-tabs">
  <a href="/" class="bottom-tab active">
    <svg class="bottom-tab-icon home-icon" width="22" height="22" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M10 20V14H14V20H19V12H22L12 3L2 12H5V20H10Z" fill="currentColor"/>
    </svg>
    <span>Головна</span>
  </a>
  <a href="/wifi" class="bottom-tab">
    <svg class="bottom-tab-icon bottom-tab-icon-svg" viewBox="0 0 24 24">
      <circle cx="12" cy="12" r="3.5"></circle>
      <line x1="12" y1="2.5" x2="12" y2="4.5"></line>
      <line x1="12" y1="19.5" x2="12" y2="21.5"></line>
      <line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line>
      <line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line>
      <line x1="2.5" y1="12" x2="4.5" y2="12"></line>
      <line x1="19.5" y1="12" x2="21.5" y2="12"></line>
      <line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line>
      <line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line>
    </svg>
    <span>Налаштування</span>
  </a>
</div>
</body>
</html>
)rawliteral";

// === Info Page ===
const char* pageInfo = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Інформація про систему</title>
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
.info-row {
  display: flex;
  justify-content: space-between;
  padding: 8px 0;
  border-bottom: 1px solid rgba(0,0,0,0.1);
}
.info-row:last-child {
  border-bottom: none;
}
.info-label {
  font-weight: 600;
  color: #555;
}
.info-value {
  color: #333;
  text-align: right;
}
.section-header {
  display: flex;
  align-items: flex-start;
  gap: 10px;
  margin-bottom: 12px;
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
a {
  display: inline-block;
  padding: 8px 16px;
  margin: 8px 4px;
  background: linear-gradient(45deg, #2196F3, #1976D2);
  color: white;
  text-decoration: none;
  border-radius: 8px;
  font-weight: 600;
  font-size: 13px;
}
</style>
</head>
<body>
<h2>ℹ️ Інформація про систему</h2>
<div style="text-align: center; margin-bottom: 16px;">
  <a href="/">🏠 Головна</a>
  <a href="/wifi">Налаштування WiFi</a>
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
      <div class="section-title">WiFi інформація</div>
      <div class="section-subtitle">Поточні мережеві параметри</div>
    </div>
  </div>
  <div class="info-row">
    <span class="info-label">SSID:</span>
    <span class="info-value" id="infoSSID">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">IP адреса:</span>
    <span class="info-value" id="infoIP">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Режим:</span>
    <span class="info-value" id="infoMode">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">mDNS:</span>
    <span class="info-value">fish.local</span>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <rect x="5" y="8" width="12" height="8" rx="2"></rect>
        <path d="M17 11h2.2a1 1 0 0 1 0 2H17"></path>
        <rect class="filled" x="7.5" y="10" width="5" height="4" rx="1"></rect>
      </svg>
    </div>
    <div>
      <div class="section-title">Батарея</div>
      <div class="section-subtitle">Стан акумулятора пристрою</div>
    </div>
  </div>
  <div class="info-row">
    <span class="info-label">Напруга:</span>
    <span class="info-value" id="infoVoltage">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Відсоток:</span>
    <span class="info-value" id="infoPercent">завантаження...</span>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="3.5"></circle>
        <path d="M12 3v2"></path>
        <path d="M12 19v2"></path>
        <path d="M21 12h-2"></path>
        <path d="M5 12H3"></path>
        <path d="M18.5 6l-1.4 1.4"></path>
        <path d="M6.9 17.6 5.5 19"></path>
        <path d="M18.5 18.5 17.1 17.1"></path>
        <path d="M6.9 6.9 5.5 5.5"></path>
      </svg>
    </div>
    <div>
      <div class="section-title">Налаштування</div>
      <div class="section-subtitle">Поточні параметри роботи</div>
    </div>
  </div>
  <div class="info-row">
    <span class="info-label">Швидкість серво:</span>
    <span class="info-value" id="infoSpeed">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Повторів годування:</span>
    <span class="info-value" id="infoRepeats">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Режим економії:</span>
    <span class="info-value" id="infoPowerSave">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Кількість розкладів:</span>
    <span class="info-value" id="infoSchedules">завантаження...</span>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <rect x="7" y="4" width="10" height="16" rx="2"></rect>
        <line x1="9" y1="8" x2="15" y2="8"></line>
        <line x1="10" y1="18" x2="14" y2="18"></line>
      </svg>
    </div>
    <div>
      <div class="section-title">Система</div>
      <div class="section-subtitle">Інформація про пристрій</div>
    </div>
  </div>
  <div class="info-row">
    <span class="info-label">Модель:</span>
    <span class="info-value">AquaFeed Hub</span>
  </div>
  <div class="info-row">
    <span class="info-label">Версія прошивки:</span>
    <span class="info-value">1.0</span>
  </div>
  <div class="info-row">
    <span class="info-label">Час роботи:</span>
    <span class="info-value" id="infoUptime">завантаження...</span>
  </div>
</div>

<script>
function updateInfo(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    document.getElementById('infoSSID').innerText = j.wifiSSID || 'не налаштовано';
    document.getElementById('infoIP').innerText = j.wifiIP || 'не підключено';
    document.getElementById('infoMode').innerText = j.isAPMode ? 'Точка доступу (AP)' : 'Станція (STA)';
    document.getElementById('infoVoltage').innerText = j.batteryVoltage.toFixed(2) + ' В';
    document.getElementById('infoPercent').innerText = Math.round(j.batteryPercent) + '%';
    document.getElementById('infoSpeed').innerText = j.speed;
    document.getElementById('infoRepeats').innerText = j.feedRepeats;
    document.getElementById('infoPowerSave').innerText = j.powerSaveMode ? 'Увімкнено' : 'Вимкнено';
    if(j.feedTimes) {
      document.getElementById('infoSchedules').innerText = j.feedTimes.length;
    } else {
      document.getElementById('infoSchedules').innerText = '2 (старий формат)';
    }
    
    // Час роботи (приблизно)
    const uptimeSeconds = Math.floor(millis() / 1000);
    const hours = Math.floor(uptimeSeconds / 3600);
    const minutes = Math.floor((uptimeSeconds % 3600) / 60);
    document.getElementById('infoUptime').innerText = hours + ' год ' + minutes + ' хв';
  });
}

// Простий лічильник часу (приблизний)
let startTime = Date.now();
function millis() {
  return Date.now() - startTime;
}

window.onload = function() {
  updateInfo();
  setInterval(updateInfo, 10000);
};
</script>
</body>
</html>
)rawliteral";

// === Handlers ===
void handleRoot(){ server.send(200,"text/html",pageIndex); }
void handleInfo(){ server.send(200,"text/html",pageInfo); }

void handleStatus(){
  batteryVoltage = readBatteryVoltage();
  batteryPercent = voltageToPercent(batteryVoltage);
  
  String json = "{\"status\":\"ok\",";
  json += "\"currentAngle\":"+String(currentAngle)+",";
  json += "\"speed\":"+String(speedSetting)+",";
  json += "\"feedRepeats\":"+String(feedRepeats)+",";
  json += "\"powerSaveMode\":"+String(powerSaveMode ? "true" : "false")+",";
  json += "\"batteryVoltage\":"+String(batteryVoltage,2)+",";
  json += "\"batteryPercent\":"+String(batteryPercent,0)+",";
  
  // Додаємо масив годувань
  json += "\"feedTimes\":[";
  for(int i = 0; i < feedTimesCount; i++) {
    if(i > 0) json += ",";
    json += "{\"h\":"+String(feedTimes[i].hour)+",\"m\":"+String(feedTimes[i].minute)+",\"r\":"+String(feedTimes[i].repeats)+"}";
  }
  json += "],";
  
  // Для сумісності додаємо старі поля
  json += "\"feedHour1\":"+String(feedHour1)+",";
  json += "\"feedMinute1\":"+String(feedMinute1)+",";
  json += "\"feedHour2\":"+String(feedHour2)+",";
  json += "\"feedMinute2\":"+String(feedMinute2)+",";
  json += "\"feedRepeats1\":"+String(feedRepeats1)+",";
  json += "\"feedRepeats2\":"+String(feedRepeats2)+",";
  json += "\"wifiSSID\":\""+savedSSID+"\",";
  json += "\"isAPMode\":"+String(isAPMode ? "true" : "false")+",";
  if(!isAPMode && WiFi.status() == WL_CONNECTED) {
    json += "\"wifiIP\":\""+WiFi.localIP().toString()+"\"";
  } else {
    json += "\"wifiIP\":\"\"";
  }
  json += "}";
  
  server.send(200,"application/json", json);
}

void handleSetAngle(){ 
  if(server.hasArg("angle") && !manualMoving) {
    moveServoFast(server.arg("angle").toInt()); 
  }
  server.send(200,"text/plain","ok"); 
}
void handleFeedNow(){ server.send(200,"text/plain","feeding"); delay(10); feedSequence(feedRepeats); }
void handleSetSpeed(){ if(server.hasArg("speed")){ speedSetting = server.arg("speed").toFloat(); preferences.putFloat("speed",speedSetting);} server.send(200,"text/plain","ok"); }
void handleSetRepeats(){ if(server.hasArg("repeats")){ feedRepeats = server.arg("repeats").toInt(); preferences.putInt("feedRepeats",feedRepeats);} server.send(200,"text/plain","ok"); }
void handleSetFeedTimes(){
  if(server.hasArg("data")) {
    // Новий формат - JSON масив
    String jsonData = server.arg("data");
    feedTimesCount = 0;
    
    // Простий парсинг JSON масиву [{h:10,m:0,r:1},...]
    int start = 0;
    while(feedTimesCount < MAX_FEED_TIMES && start < jsonData.length()) {
      int objStart = jsonData.indexOf('{', start);
      if(objStart == -1) break;
      
      int objEnd = jsonData.indexOf('}', objStart);
      if(objEnd == -1) break;
      
      String obj = jsonData.substring(objStart + 1, objEnd);
      int h = 10, m = 0, r = 1;
      
      // Парсимо h, m, r
      int hPos = obj.indexOf("\"h\":");
      if(hPos == -1) hPos = obj.indexOf("h:");
      if(hPos != -1) {
        int hEnd = obj.indexOf(',', hPos);
        if(hEnd == -1) hEnd = obj.length();
        h = obj.substring(obj.indexOf(':', hPos) + 1, hEnd).toInt();
      }
      
      int mPos = obj.indexOf("\"m\":");
      if(mPos == -1) mPos = obj.indexOf("m:");
      if(mPos != -1) {
        int mEnd = obj.indexOf(',', mPos);
        if(mEnd == -1) mEnd = obj.length();
        m = obj.substring(obj.indexOf(':', mPos) + 1, mEnd).toInt();
      }
      
      int rPos = obj.indexOf("\"r\":");
      if(rPos == -1) rPos = obj.indexOf("r:");
      if(rPos != -1) {
        int rEnd = obj.indexOf('}', rPos);
        if(rEnd == -1) rEnd = obj.length();
        r = obj.substring(obj.indexOf(':', rPos) + 1, rEnd).toInt();
      }
      
      feedTimes[feedTimesCount].hour = h;
      feedTimes[feedTimesCount].minute = m;
      feedTimes[feedTimesCount].repeats = r;
      feedTimes[feedTimesCount].done = false;
      feedTimesCount++;
      
      start = objEnd + 1;
    }
    
    // Зберігаємо в Preferences
    preferences.putInt("feedTimesCount", feedTimesCount);
    char key[20];
    for(int i = 0; i < feedTimesCount; i++) {
      sprintf(key, "feedH%d", i);
      preferences.putInt(key, feedTimes[i].hour);
      sprintf(key, "feedM%d", i);
      preferences.putInt(key, feedTimes[i].minute);
      sprintf(key, "feedR%d", i);
      preferences.putInt(key, feedTimes[i].repeats);
    }
    
    // Для сумісності зберігаємо перші два
    if(feedTimesCount > 0) {
      feedHour1 = feedTimes[0].hour;
      feedMinute1 = feedTimes[0].minute;
      feedRepeats1 = feedTimes[0].repeats;
    }
    if(feedTimesCount > 1) {
      feedHour2 = feedTimes[1].hour;
      feedMinute2 = feedTimes[1].minute;
      feedRepeats2 = feedTimes[1].repeats;
    }
  } else {
    // Старий формат для сумісності
    feedHour1 = server.arg("h1").toInt(); feedMinute1 = server.arg("m1").toInt();
    feedHour2 = server.arg("h2").toInt(); feedMinute2 = server.arg("m2").toInt();
    feedRepeats1 = server.arg("r1").toInt(); feedRepeats2 = server.arg("r2").toInt();
    
    feedTimesCount = 2;
    feedTimes[0] = {feedHour1, feedMinute1, feedRepeats1, false};
    feedTimes[1] = {feedHour2, feedMinute2, feedRepeats2, false};
    
    preferences.putInt("feedTimesCount", 2);
    char key[20];
    sprintf(key, "feedH%d", 0);
    preferences.putInt(key, feedHour1);
    sprintf(key, "feedM%d", 0);
    preferences.putInt(key, feedMinute1);
    sprintf(key, "feedR%d", 0);
    preferences.putInt(key, feedRepeats1);
    sprintf(key, "feedH%d", 1);
    preferences.putInt(key, feedHour2);
    sprintf(key, "feedM%d", 1);
    preferences.putInt(key, feedMinute2);
    sprintf(key, "feedR%d", 1);
    preferences.putInt(key, feedRepeats2);
  }
  
  preferences.putInt("feedHour1",feedHour1);
  preferences.putInt("feedMinute1",feedMinute1);
  preferences.putInt("feedHour2",feedHour2);
  preferences.putInt("feedMinute2",feedMinute2);
  preferences.putInt("feedRepeats1",feedRepeats1);
  preferences.putInt("feedRepeats2",feedRepeats2);
  updateActivity();
  server.send(200,"text/plain","ok");
}

void handleSetPowerMode(){
  if(server.hasArg("enabled")){
    powerSaveMode = server.arg("enabled") == "true";
    preferences.putBool("powerSaveMode", powerSaveMode);
    updateActivity();
  }
  server.send(200,"text/plain","ok");
}


// === Setup ===
void setup(){
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);

  mg996r.setPeriodHertz(50);
  mg996r.attach(SERVO_PIN,600,2400);
  mg996r.write(currentAngle);

  preferences.begin("feeder", false);
  speedSetting = preferences.getFloat("speed",20.0);
  feedRepeats = preferences.getInt("feedRepeats",1);
  powerSaveMode = preferences.getBool("powerSaveMode", true);
  
  // Завантажуємо масив годувань
  feedTimesCount = preferences.getInt("feedTimesCount", 0);
  if(feedTimesCount <= 0 || feedTimesCount > MAX_FEED_TIMES) {
    feedTimesCount = 0;

    int storedH1 = preferences.getInt("feedHour1", -1);
    int storedM1 = preferences.getInt("feedMinute1", -1);
    int storedR1 = preferences.getInt("feedRepeats1", -1);

    if(storedH1 < 0) storedH1 = 10;
    if(storedM1 < 0) storedM1 = 0;
    if(storedR1 < 0) storedR1 = 1;

    feedTimes[feedTimesCount++] = {storedH1, storedM1, storedR1, false};

    if(preferences.isKey("feedHour2") && preferences.isKey("feedMinute2")) {
      int storedH2 = preferences.getInt("feedHour2", storedH1);
      int storedM2 = preferences.getInt("feedMinute2", storedM1);
      int storedR2 = preferences.getInt("feedRepeats2", storedR1);
      feedTimes[feedTimesCount++] = {storedH2, storedM2, storedR2, false};
    }
  } else {
    // Завантажуємо збережені годування
    char key[20];
    for(int i = 0; i < feedTimesCount; i++) {
      sprintf(key, "feedH%d", i);
      feedTimes[i].hour = preferences.getInt(key, 10);
      sprintf(key, "feedM%d", i);
      feedTimes[i].minute = preferences.getInt(key, 0);
      sprintf(key, "feedR%d", i);
      feedTimes[i].repeats = preferences.getInt(key, 1);
      feedTimes[i].done = false;
    }
  }

  // Синхронізуємо значення для сумісності зі старим кодом
  if(feedTimesCount > 0) {
    feedHour1 = feedTimes[0].hour;
    feedMinute1 = feedTimes[0].minute;
    feedRepeats1 = feedTimes[0].repeats;
  }
  if(feedTimesCount > 1) {
    feedHour2 = feedTimes[1].hour;
    feedMinute2 = feedTimes[1].minute;
    feedRepeats2 = feedTimes[1].repeats;
  } else {
    feedHour2 = 0;
    feedMinute2 = 0;
    feedRepeats2 = 1;
  }
  
  // Ініціалізуємо час останньої активності
  lastActivity = millis();

  // Ініціалізуємо WiFi
  initWiFi(preferences);

  server.on("/", handleRoot);
  server.on("/info", handleInfo);
  server.on("/api/status", handleStatus);
  server.on("/api/setAngle", handleSetAngle);
  server.on("/api/feedNow", handleFeedNow);
  server.on("/api/setSpeed", handleSetSpeed);
  server.on("/api/setRepeats", handleSetRepeats);
  server.on("/api/setFeedTimes", handleSetFeedTimes);
  server.on("/api/setPowerMode", handleSetPowerMode);
  
  // Налаштування WiFi обробників
  setupWiFiHandlers(server, preferences);
  
  server.begin();
  Serial.println("HTTP server started");
}

// === Loop ===
void loop(){
  server.handleClient();
  bool buttonState=digitalRead(BUTTON_PIN);
  if(lastButtonState==HIGH && buttonState==LOW && !manualMoving){ feedSequence(); }
  lastButtonState = buttonState;

  // --- Automatic feeding by schedule ---
  time_t now = time(nullptr);
  struct tm timeinfo;
  if (localtime_r(&now, &timeinfo)) {
    int curHour = timeinfo.tm_hour;
    int curMinute = timeinfo.tm_min;

    // Перевіряємо всі годування з масиву
    for(int i = 0; i < feedTimesCount; i++) {
      if (curHour == feedTimes[i].hour && curMinute == feedTimes[i].minute && !feedTimes[i].done) {
        Serial.printf("Auto feeding (slot %d) %02d:%02d, repeats: %d\n", i+1, curHour, curMinute, feedTimes[i].repeats);
        feedSequence(feedTimes[i].repeats);
        feedTimes[i].done = true;
      }
      
      // Скидаємо прапорець коли хвилина змінилася
      if (curHour != feedTimes[i].hour || curMinute != feedTimes[i].minute) {
        feedTimes[i].done = false;
      }
    }
    
    // Для сумісності зі старим кодом
    if (curHour == feedHour1 && curMinute == feedMinute1 && !feed1Done) {
      Serial.printf("Auto feeding (slot 1) %02d:%02d, repeats: %d\n", curHour, curMinute, feedRepeats1);
      feedSequence(feedRepeats1);
      feed1Done = true;
    }
    if (curHour == feedHour2 && curMinute == feedMinute2 && !feed2Done) {
      Serial.printf("Auto feeding (slot 2) %02d:%02d, repeats: %d\n", curHour, curMinute, feedRepeats2);
      feedSequence(feedRepeats2);
      feed2Done = true;
    }
    if (curHour != feedHour1 || curMinute != feedMinute1) feed1Done = false;
    if (curHour != feedHour2 || curMinute != feedMinute2) feed2Done = false;
  }
}