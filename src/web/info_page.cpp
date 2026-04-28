#include "../web_pages.h"
#include "version_info.h"

const char* pageInfo = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Інформація про систему</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
)rawliteral"
#include "shared/common_body_base_styles.inc"
#include "shared/common_card_styles.inc"
R"rawliteral(
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
)rawliteral"
#include "shared/common_section_styles.inc"
#include "shared/common_hero_styles.inc"
#include "shared/common_bottom_nav_styles.inc"
R"rawliteral(
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
body {
  padding-bottom: 75px;
}
a {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 12px 24px;
  font-size: 13px;
  margin: 8px 4px;
  border: none;
  border-radius: 999px;
  background: #111;
  color: #fff;
  font-weight: 600;
  letter-spacing: 0.2px;
  cursor: pointer;
  text-decoration: none;
  transition: transform 0.15s ease, background 0.2s ease;
}
a:hover {
  transform: translateY(-1px);
  box-shadow: 0 14px 22px rgba(0,0,0,0.2);
}
a:active {
  transform: translateY(0);
  box-shadow: 0 6px 14px rgba(0,0,0,0.16);
}
</style>
</head>
<body>
<div id="toast" class="toast">Збережено</div>
<div class="hero-header">
)rawliteral"
#include "shared/hero_illustration.inc"
R"rawliteral(
  <div class="app-heading">
    <div class="app-title">AquaFeed Control</div>
    <div class="app-subtitle">Інформація про систему</div>
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
    <span class="info-label">OLED дисплей:</span>
    <span class="info-value" id="infoDisplayEnabled">завантаження...</span>
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
    <span class="info-value">)rawliteral" AQUAFEED_FIRMWARE_VERSION R"rawliteral(</span>
  </div>
  <div class="info-row">
    <span class="info-label">Час роботи:</span>
    <span class="info-value" id="infoUptime">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">CPU частота:</span>
    <span class="info-value" id="infoCpuFreq">завантаження...</span>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect>
        <line x1="9" y1="9" x2="15" y2="9"></line>
        <line x1="9" y1="15" x2="15" y2="15"></line>
      </svg>
    </div>
    <div>
      <div class="section-title">Пам'ять та кеш</div>
      <div class="section-subtitle">Використання ресурсів системи</div>
    </div>
  </div>
  <div class="info-row">
    <span class="info-label">Вільна пам'ять:</span>
    <span class="info-value" id="infoFreeHeap">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Використано пам'яті:</span>
    <span class="info-value" id="infoUsedHeap">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Загальна пам'ять:</span>
    <span class="info-value" id="infoTotalHeap">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Макс. блок:</span>
    <span class="info-value" id="infoMaxAlloc">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Мін. вільна (завжди):</span>
    <span class="info-value" id="infoMinFree">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Розмір кешу:</span>
    <span class="info-value" id="infoCacheSize">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Вік кешу:</span>
    <span class="info-value" id="infoCacheAge">завантаження...</span>
  </div>
  <div class="info-row">
    <span class="info-label">Статус кешу:</span>
    <span class="info-value" id="infoCacheStatus">завантаження...</span>
  </div>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
function infoIsEn() { return isStoredUiLangEn(); }
function applyInfoLanguage() {
  document.title = infoIsEn() ? 'System information' : 'Інформація про систему';
  const infoLblUk = ['SSID:', 'IP адреса:', 'Режим:', 'mDNS:', 'Напруга:', 'Відсоток:', 'Швидкість серво:', 'Повторів годування:', 'Режим економії:', 'OLED дисплей:', 'Кількість розкладів:', 'Модель:', 'Версія прошивки:', 'Час роботи:', 'CPU частота:', 'Вільна пам\'ять:', 'Використано пам\'яті:', 'Загальна пам\'ять:', 'Макс. блок:', 'Мін. вільна (завжди):', 'Розмір кешу:', 'Вік кешу:', 'Статус кешу:'];
  const infoLblEn = ['SSID:', 'IP address:', 'Mode:', 'mDNS:', 'Voltage:', 'Percent:', 'Servo speed:', 'Feeding repeats:', 'Power save:', 'OLED display:', 'Schedules:', 'Model:', 'Firmware:', 'Uptime:', 'CPU frequency:', 'Free memory:', 'Used memory:', 'Total memory:', 'Max block:', 'Min free (always):', 'Cache size:', 'Cache age:', 'Cache status:'];
  document.querySelectorAll('.info-label').forEach((el, i) => {
    if (i < infoLblUk.length) el.textContent = infoIsEn() ? infoLblEn[i] : infoLblUk[i];
  });
  const loadTxt = infoIsEn() ? 'loading...' : 'завантаження...';
  document.querySelectorAll('.info-value').forEach((el) => {
    const tx = el.textContent.trim();
    if (tx === 'завантаження...' || tx === 'loading...') el.textContent = loadTxt;
  });
  const toastInfo = document.getElementById('toast');
  if (toastInfo) toastInfo.textContent = infoIsEn() ? 'Saved' : 'Збережено';
  const tabs = document.querySelectorAll('.bottom-tab span');
  const subtitles = document.querySelectorAll('.section-subtitle');
  const sections = document.querySelectorAll('.section-title');
  const appSubtitle = document.querySelector('.app-subtitle');
  if (appSubtitle) appSubtitle.textContent = infoIsEn() ? 'System information' : 'Інформація про систему';
  if (tabs[0]) tabs[0].textContent = infoIsEn() ? 'Home' : 'Головна';
  if (tabs[1]) tabs[1].textContent = infoIsEn() ? 'Info' : 'Інформація';
  if (tabs[2]) tabs[2].textContent = infoIsEn() ? 'Settings' : 'Налаштування';
  if (sections[0]) sections[0].textContent = infoIsEn() ? 'WiFi information' : 'WiFi інформація';
  if (subtitles[0]) subtitles[0].textContent = infoIsEn() ? 'Current network parameters' : 'Поточні мережеві параметри';
  if (sections[1]) sections[1].textContent = infoIsEn() ? 'Battery' : 'Батарея';
  if (subtitles[1]) subtitles[1].textContent = infoIsEn() ? 'Device battery state' : 'Стан акумулятора пристрою';
  if (sections[2]) sections[2].textContent = infoIsEn() ? 'Settings' : 'Налаштування';
  if (subtitles[2]) subtitles[2].textContent = infoIsEn() ? 'Current runtime parameters' : 'Поточні параметри роботи';
  if (sections[3]) sections[3].textContent = infoIsEn() ? 'System' : 'Система';
  if (subtitles[3]) subtitles[3].textContent = infoIsEn() ? 'Device information' : 'Інформація про пристрій';
  if (sections[4]) sections[4].textContent = infoIsEn() ? 'Memory and cache' : 'Пам\'ять та кеш';
  if (subtitles[4]) subtitles[4].textContent = infoIsEn() ? 'System resource usage' : 'Використання ресурсів системи';
  const heroFishInfo = document.querySelector('.hero-svg-fish');
  if (heroFishInfo) heroFishInfo.setAttribute('aria-label', infoIsEn() ? 'Stylized fish' : 'Стилізована рибка');
}
function updateInfo(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    document.getElementById('infoSSID').innerText = j.wifiSSID || (infoIsEn() ? 'not set' : 'не налаштовано');
    document.getElementById('infoIP').innerText = j.wifiIP || (infoIsEn() ? 'not connected' : 'не підключено');
    document.getElementById('infoMode').innerText = j.isAPMode ? (infoIsEn() ? 'Access Point (AP)' : 'Точка доступу (AP)') : (infoIsEn() ? 'Station (STA)' : 'Станція (STA)');
    if (typeof j.batteryVoltage === 'number' && Number.isFinite(j.batteryVoltage)) {
      document.getElementById('infoVoltage').innerText = j.batteryVoltage.toFixed(2) + (infoIsEn() ? ' V' : ' В');
    } else {
      document.getElementById('infoVoltage').innerText = '-- В';
    }
    const infoPercentEl = document.getElementById('infoPercent');
    let infoPercentVal = Number(j.batteryPercent);
    if (!Number.isFinite(infoPercentVal)) {
      infoPercentVal = voltageToPercentClient(Number(j.batteryVoltage));
    }
    if (Number.isFinite(infoPercentVal)) {
      infoPercentEl.innerText = Math.round(infoPercentVal) + '%';
    } else {
      infoPercentEl.innerText = '--%';
    }
    document.getElementById('infoSpeed').innerText = j.speed;
    document.getElementById('infoRepeats').innerText = j.feedRepeats;
    document.getElementById('infoPowerSave').innerText = j.powerSaveMode ? (infoIsEn() ? 'Enabled' : 'Увімкнено') : (infoIsEn() ? 'Disabled' : 'Вимкнено');
    document.getElementById('infoDisplayEnabled').innerText = j.displayEnabled ? (infoIsEn() ? 'Enabled' : 'Увімкнено') : (infoIsEn() ? 'Disabled' : 'Вимкнено');
    if(j.feedTimes) {
      document.getElementById('infoSchedules').innerText = j.feedTimes.length;
    } else {
      document.getElementById('infoSchedules').innerText = infoIsEn() ? '2 (legacy format)' : '2 (старий формат)';
    }
    
    // Час роботи (приблизно)
    const uptimeSeconds = Math.floor(millis() / 1000);
    const hours = Math.floor(uptimeSeconds / 3600);
    const minutes = Math.floor((uptimeSeconds % 3600) / 60);
    document.getElementById('infoUptime').innerText = infoIsEn() ? (hours + ' h ' + minutes + ' m') : (hours + ' год ' + minutes + ' хв');
    
    // CPU частота
    if (typeof j.cpuFrequency === 'number') {
      document.getElementById('infoCpuFreq').innerText = j.cpuFrequency + ' MHz';
    } else {
      document.getElementById('infoCpuFreq').innerText = '-- MHz';
    }
    
    // Пам'ять
    function formatBytes(bytes) {
      if (bytes >= 1024) {
        return (bytes / 1024).toFixed(2) + ' KB';
      }
      return bytes + ' B';
    }
    
    if (typeof j.memoryFreeHeap === 'number') {
      document.getElementById('infoFreeHeap').innerText = formatBytes(j.memoryFreeHeap);
    } else {
      document.getElementById('infoFreeHeap').innerText = '--';
    }
    
    if (typeof j.memoryUsedHeap === 'number') {
      document.getElementById('infoUsedHeap').innerText = formatBytes(j.memoryUsedHeap);
    } else {
      document.getElementById('infoUsedHeap').innerText = '--';
    }
    
    if (typeof j.memoryTotalHeap === 'number') {
      document.getElementById('infoTotalHeap').innerText = formatBytes(j.memoryTotalHeap);
    } else {
      document.getElementById('infoTotalHeap').innerText = '--';
    }
    
    if (typeof j.memoryMaxAllocHeap === 'number') {
      document.getElementById('infoMaxAlloc').innerText = formatBytes(j.memoryMaxAllocHeap);
    } else {
      document.getElementById('infoMaxAlloc').innerText = '--';
    }
    
    if (typeof j.memoryMinFreeHeap === 'number') {
      document.getElementById('infoMinFree').innerText = formatBytes(j.memoryMinFreeHeap);
    } else {
      document.getElementById('infoMinFree').innerText = '--';
    }
    
    // Кеш
    if (typeof j.cacheSize === 'number') {
      document.getElementById('infoCacheSize').innerText = formatBytes(j.cacheSize);
    } else {
      document.getElementById('infoCacheSize').innerText = '--';
    }
    
    if (typeof j.cacheAge === 'number') {
      const ageMs = j.cacheAge;
      if (ageMs < 1000) {
        document.getElementById('infoCacheAge').innerText = ageMs + ' ms';
      } else {
        document.getElementById('infoCacheAge').innerText = (ageMs / 1000).toFixed(2) + ' s';
      }
    } else {
      document.getElementById('infoCacheAge').innerText = '--';
    }
    
    if (typeof j.cacheValid === 'boolean' || j.cacheValid === 'true' || j.cacheValid === 'false') {
      const isValid = j.cacheValid === true || j.cacheValid === 'true';
      document.getElementById('infoCacheStatus').innerText = isValid ? (infoIsEn() ? '✅ Active' : '✅ Активний') : (infoIsEn() ? '❌ Inactive' : '❌ Неактивний');
    } else {
      document.getElementById('infoCacheStatus').innerText = '--';
    }
  });
}

// Простий лічильник часу (приблизний)
let startTime = Date.now();
function millis() {
  return Date.now() - startTime;
}

function showToast(text = (infoIsEn() ? 'Saved' : 'Збережено')) {
  showToastMessage(text);
}

window.onload = function() {
  applyInfoLanguage();
  updateInfo();
  setInterval(updateInfo, 10000);
  setActiveBottomTab();
};
</script>

)rawliteral"
#include "shared/bottom_tabs_info.inc"
R"rawliteral(
</body>
</html>
)rawliteral";
