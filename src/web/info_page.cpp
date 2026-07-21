// Info page ("/info") — read-only device info, served directly by the ESP32
// (same fallback rationale as home_page.cpp).
const char* pageInfo = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>AquaFeed — Інформація</title>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<style>
)rawliteral"
#include "shared/common_body_base_styles.inc"
R"rawliteral(
)rawliteral"
#include "shared/common_card_styles.inc"
R"rawliteral(
)rawliteral"
#include "shared/common_section_styles.inc"
R"rawliteral(
.section-header { margin-bottom: 16px; }
)rawliteral"
#include "shared/common_hero_styles.inc"
R"rawliteral(
.info-row {
  display: flex;
  justify-content: space-between;
  gap: 12px;
  padding: 6px 0;
  border-bottom: 1px solid rgba(0,0,0,0.06);
  font-size: 13px;
}
.info-row:last-child { border-bottom: none; }
.info-label { color: #666; }
.info-value { color: #222; font-weight: 600; text-align: right; word-break: break-all; }
)rawliteral"
#include "shared/common_bottom_nav_styles.inc"
R"rawliteral(
body { padding-bottom: calc(75px + env(safe-area-inset-bottom, 0px)); }
</style>
</head>
<body>
<div class="hero-header">
)rawliteral"
#include "shared/hero_illustration.inc"
R"rawliteral(
  <div class="app-heading">
    <div class="app-title">AquaFeed</div>
    <div class="app-subtitle" id="infoSubtitle">Інформація про пристрій</div>
  </div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 17a2 2 0 1 0 0-4"></path>
        <path d="M7 10V8a5 5 0 0 1 10 0v2"></path>
        <rect x="5" y="10" width="14" height="10" rx="2"></rect>
      </svg>
    </div>
    <div>
      <div class="section-title" id="fwSectionTitle">Прошивка</div>
      <div class="section-subtitle" id="fwSectionSubtitle">Версія та збірка</div>
    </div>
  </div>
  <div class="info-row"><span class="info-label" id="lblFwVersion">Версія</span><span class="info-value" id="valFwVersion">--</span></div>
  <div class="info-row"><span class="info-label" id="lblBuildDate">Дата збірки</span><span class="info-value" id="valBuildDate">--</span></div>
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
      <div class="section-title" id="netSectionTitle">Мережа</div>
      <div class="section-subtitle" id="netSectionSubtitle">WiFi та адреса пристрою</div>
    </div>
  </div>
  <div class="info-row"><span class="info-label" id="lblWifiSsid">Мережа</span><span class="info-value" id="valWifiSsid">--</span></div>
  <div class="info-row"><span class="info-label" id="lblWifiIp">IP-адреса</span><span class="info-value" id="valWifiIp">--</span></div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <rect x="2" y="7" width="16" height="10" rx="2" ry="2"></rect>
        <line x1="22" y1="11" x2="22" y2="13"></line>
      </svg>
    </div>
    <div>
      <div class="section-title" id="battSectionTitle">Батарея</div>
      <div class="section-subtitle" id="battSectionSubtitle">Напруга та заряд</div>
    </div>
  </div>
  <div class="info-row"><span class="info-label" id="lblVoltage">Напруга</span><span class="info-value" id="valVoltage">--</span></div>
  <div class="info-row"><span class="info-label" id="lblPercent">Заряд</span><span class="info-value" id="valPercent">--</span></div>
  <div class="info-row"><span class="info-label" id="lblIsCharging">Заряджається</span><span class="info-value" id="valIsCharging">--</span></div>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="10"></circle>
        <path d="M12 7v5"></path>
        <path d="M12 12l3 2"></path>
      </svg>
    </div>
    <div>
      <div class="section-title" id="runtimeSectionTitle">Стан роботи</div>
      <div class="section-subtitle" id="runtimeSectionSubtitle">Час роботи та сон</div>
    </div>
  </div>
  <div class="info-row"><span class="info-label" id="lblUptime">Час роботи</span><span class="info-value" id="valUptime">--</span></div>
  <div class="info-row"><span class="info-label" id="lblDeviceTime">Час пристрою</span><span class="info-value" id="valDeviceTime">--</span></div>
  <div class="info-row"><span class="info-label" id="lblCpuFreq">Частота CPU</span><span class="info-value" id="valCpuFreq">--</span></div>
  <div class="info-row"><span class="info-label" id="lblSleepReason">Причина сну</span><span class="info-value" id="valSleepReason">--</span></div>
  <div class="info-row"><span class="info-label" id="lblFreeHeap">Вільна пам'ять</span><span class="info-value" id="valFreeHeap">--</span></div>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
let infoLang = getStoredUiLang();
function infoIsEn() { return infoLang === 'en'; }

function formatUptime(seconds) {
  if (typeof seconds !== 'number') return '--';
  const h = Math.floor(seconds / 3600);
  const m = Math.floor((seconds % 3600) / 60);
  return h + (infoIsEn() ? 'h ' : 'г ') + m + (infoIsEn() ? 'm' : 'хв');
}

function applyInfoLanguage() {
  const setText = (id, en, uk) => {
    const el = document.getElementById(id);
    if (el) el.textContent = infoIsEn() ? en : uk;
  };
  setText('infoSubtitle', 'Device information', 'Інформація про пристрій');
  setText('fwSectionTitle', 'Firmware', 'Прошивка');
  setText('fwSectionSubtitle', 'Version and build', 'Версія та збірка');
  setText('lblFwVersion', 'Version', 'Версія');
  setText('lblBuildDate', 'Build date', 'Дата збірки');
  setText('netSectionTitle', 'Network', 'Мережа');
  setText('netSectionSubtitle', 'WiFi and device address', 'WiFi та адреса пристрою');
  setText('lblWifiSsid', 'Network', 'Мережа');
  setText('lblWifiIp', 'IP address', 'IP-адреса');
  setText('battSectionTitle', 'Battery', 'Батарея');
  setText('battSectionSubtitle', 'Voltage and charge', 'Напруга та заряд');
  setText('lblVoltage', 'Voltage', 'Напруга');
  setText('lblPercent', 'Charge', 'Заряд');
  setText('lblIsCharging', 'Charging', 'Заряджається');
  setText('runtimeSectionTitle', 'Runtime', 'Стан роботи');
  setText('runtimeSectionSubtitle', 'Uptime and sleep', 'Час роботи та сон');
  setText('lblUptime', 'Uptime', 'Час роботи');
  setText('lblDeviceTime', 'Device time', 'Час пристрою');
  setText('lblCpuFreq', 'CPU frequency', 'Частота CPU');
  setText('lblSleepReason', 'Sleep reason', 'Причина сну');
  setText('lblFreeHeap', 'Free memory', "Вільна пам'ять");
  const tabs = document.querySelectorAll('.bottom-tab span');
  if (tabs[0]) tabs[0].textContent = infoIsEn() ? 'Home' : 'Головна';
  if (tabs[1]) tabs[1].textContent = infoIsEn() ? 'Info' : 'Інформація';
  if (tabs[2]) tabs[2].textContent = infoIsEn() ? 'Settings' : 'Налаштування';
  const heroFish = document.querySelector('.hero-svg-fish');
  if (heroFish) heroFish.setAttribute('aria-label', infoIsEn() ? 'Stylized fish' : 'Стилізована рибка');
  updateStatus();
}

function updateStatus() {
  fetch('/api/status').then(expectOk).then(r => r.json()).then(j => {
    const set = (id, val) => { const el = document.getElementById(id); if (el) el.textContent = val; };
    set('valFwVersion', j.firmwareVersion || '--');
    set('valBuildDate', (j.buildDate || '--') + (j.buildTime ? (' ' + j.buildTime) : ''));
    set('valWifiSsid', j.isAPMode ? (infoIsEn() ? 'AP mode' : 'Режим AP') : (j.wifiSSID || '--'));
    set('valWifiIp', j.wifiIP || '--');
    set('valVoltage', typeof j.batteryVoltage === 'number' ? j.batteryVoltage.toFixed(2) + ' V' : '--');
    set('valPercent', typeof j.batteryPercent === 'number' ? j.batteryPercent + '%' : '--');
    set('valIsCharging', j.isCharging ? (infoIsEn() ? 'Yes' : 'Так') : (infoIsEn() ? 'No' : 'Ні'));
    set('valUptime', formatUptime(j.uptimeSeconds));
    set('valDeviceTime', j.currentTime || '--');
    set('valCpuFreq', typeof j.cpuFrequency === 'number' ? j.cpuFrequency + ' MHz' : '--');
    set('valSleepReason', j.sleepReason || '--');
    set('valFreeHeap', typeof j.memoryFreeHeap === 'number' ? Math.round(j.memoryFreeHeap / 1024) + ' KB' : '--');
  }).catch(() => {});
}

window.onload = function() {
  applyInfoLanguage();
  setInterval(updateStatus, 5000);
};
document.addEventListener('DOMContentLoaded', function() {
  setActiveBottomTab();
});
</script>

)rawliteral"
#include "shared/bottom_tabs_info.inc"
R"rawliteral(</body>
</html>
)rawliteral";
