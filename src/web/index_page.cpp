#include "../web_pages.h"

const char* pageIndex = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
<title>AquaFeed Control</title>
<style>
)rawliteral"
#include "shared/common_body_base_styles.inc"
R"rawliteral(
.row {margin-bottom: 8px;}
label {display: block; font-weight: 600; margin-bottom: 4px; color: #2c3e50; font-size: 13px;}
input[type=range], select {width: 95%; padding: 6px; border-radius: 6px; border: 1px solid #ddd; font-size: 13px;}
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
.note-text {
  font-size: 12px;
  color: #6b7280;
  line-height: 1.5;
  margin: 10px 0 4px;
}
.feed-block {
  margin-bottom: 8px;
  padding: 8px;
  background: rgba(0, 0, 0, 0.03);
  border-radius: 6px;
}
)rawliteral"
#include "shared/common_card_styles.inc"
R"rawliteral(
.flex-row {
  display: flex;
  gap: 4px;
  align-items: center;
  margin-bottom: 6px;
  flex-wrap: wrap;
}
.flex-row span {font-weight: 500; color: #555; font-size: 12px;}
.flex-row input, .flex-row select {width: auto; min-width: 20px; font-size: 12px;}
.feed-block .flex-row {
  flex-wrap: nowrap;
}
.feed-block .feed-time {
  width: 84px;
  min-width: 84px;
}
.feed-block .feed-repeats {
  width: 64px;
  min-width: 64px;
}
.toast {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  background: linear-gradient(180deg, rgba(15,23,42,0.58), rgba(15,23,42,0.42));
  color: rgba(248,250,252,0.96);
  padding: 12px 16px;
  border-radius: 0 0 12px 12px;
  border-bottom: 1px solid rgba(255,255,255,0.18);
  box-shadow: 0 10px 26px rgba(0,0,0,0.16);
  backdrop-filter: blur(10px) saturate(125%);
  -webkit-backdrop-filter: blur(10px) saturate(125%);
  font-weight: 600;
  font-size: 14px;
  letter-spacing: 0.3px;
  text-align: center;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  z-index: 1100;
  opacity: 0;
  transform: translateY(-14px) scale(0.985);
  transition: opacity 0.42s cubic-bezier(0.22, 1, 0.36, 1), transform 0.42s cubic-bezier(0.22, 1, 0.36, 1);
  pointer-events: none;
}
.toast-icon {
  width: 18px;
  height: 18px;
  border-radius: 50%;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: rgba(255,255,255,0.2);
  border: 1px solid rgba(255,255,255,0.35);
  font-size: 12px;
  font-weight: 700;
  line-height: 1;
  flex: 0 0 auto;
}
.toast-text {
  display: inline-block;
}
.toast.show {
  opacity: 1;
  transform: translateY(0) scale(1);
}
#btnFeedNow {
  position: relative;
  overflow: hidden;
  transition: transform 0.18s ease, box-shadow 0.25s ease, opacity 0.2s ease;
}
#btnFeedNow:not(:disabled):hover {
  transform: translateY(-1px);
}
#btnFeedNow.is-feeding {
  animation: feedNowPulse 1.1s ease-in-out infinite;
}
#btnFeedNow.is-feeding::after {
  content: '';
  position: absolute;
  top: 0;
  left: -45%;
  width: 45%;
  height: 100%;
  background: linear-gradient(105deg, rgba(255,255,255,0), rgba(255,255,255,0.28), rgba(255,255,255,0));
  animation: feedNowSweep 0.95s linear infinite;
}
@keyframes feedNowPulse {
  0% { box-shadow: 0 0 0 0 rgba(244,67,54,0.35); }
  70% { box-shadow: 0 0 0 10px rgba(244,67,54,0); }
  100% { box-shadow: 0 0 0 0 rgba(244,67,54,0); }
}
@keyframes feedNowSweep {
  from { left: -45%; }
  to { left: 115%; }
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
.low-battery-alert {
  width: 100%;
  margin-top: 10px;
  display: none;
  align-items: center;
  justify-content: center;
  gap: 8px;
  padding: 10px 12px;
  border-radius: 10px;
  border: 1px solid rgba(198, 40, 40, 0.35);
  background: rgba(211, 47, 47, 0.12);
  color: #b71c1c;
  font-size: 13px;
  font-weight: 700;
  letter-spacing: 0.15px;
}
.low-battery-alert.is-low {
  border: 1px solid rgba(198, 40, 40, 0.35);
  background: rgba(211, 47, 47, 0.12);
  color: #b71c1c;
}
.low-battery-alert.is-low .low-battery-alert-icon {
  background: #d32f2f;
  color: #fff;
}
.low-battery-alert.is-charging {
  border: 1px solid rgba(46, 125, 50, 0.35);
  background: rgba(76, 175, 80, 0.14);
  color: #1b5e20;
}
.low-battery-alert.is-charging .low-battery-alert-icon {
  background: #2e7d32;
  color: #fff;
}
.low-battery-alert.is-visible {
  display: inline-flex;
}
.low-battery-alert-icon {
  width: 18px;
  height: 18px;
  border-radius: 50%;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: #d32f2f;
  color: #fff;
  font-size: 12px;
  font-weight: 800;
  line-height: 1;
  overflow: hidden;
  flex-shrink: 0;
  font-family: system-ui, -apple-system, 'Segoe UI', sans-serif;
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
@keyframes gauge-charge-fill {
  0%   { stroke-dashoffset: var(--charge-full, 345.58); }
  62%  { stroke-dashoffset: var(--charge-target, 0); }
  80%  { stroke-dashoffset: var(--charge-target, 0); }
  100% { stroke-dashoffset: var(--charge-full, 345.58); }
}
.battery-gauge-charging {
  animation: gauge-charge-fill 2.4s cubic-bezier(0.4, 0, 0.2, 1) infinite;
  transition: none !important;
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
  display: grid;
  justify-items: center;
  row-gap: 6px;
  text-align: center;
  border: none;
  outline: none;
  background: transparent;
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
)rawliteral"
#include "shared/common_section_styles.inc"
#include "shared/common_bottom_nav_styles.inc"
R"rawliteral(
body {
  padding-bottom: calc(75px + env(safe-area-inset-bottom, 0px));
}
)rawliteral"
#include "shared/common_hero_styles.inc"
R"rawliteral(
</style>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-annotation@1.3.1"></script>
</head>
<body>
<div id="toast" class="toast">
  <span class="toast-icon" aria-hidden="true">i</span>
  <span id="toastText" class="toast-text">Збережено</span>
</div>
<div class="hero-header">
)rawliteral"
#include "shared/hero_illustration.inc"
R"rawliteral(
  <div class="app-heading">
    <div class="app-title">AquaFeed Control</div>
    <div class="app-subtitle">Розумна годівниця</div>
  </div>
</div>

<div id="deepSleepHelpBanner" class="card" style="margin-top: 0; padding: 10px 14px; background: #f0f7ff; border: 1px solid #c5d9f0; font-size: 13px; color: #374151; line-height: 1.4;"></div>

<div class="card battery-card" style="margin-top: 0;">
  <div class="battery-info-row">
    <div class="battery-gauge-wrapper">
      <svg id="batteryGauge" class="battery-gauge-svg" viewBox="0 0 260 160">
        <path id="gaugeBg" d="M 20 140 A 110 110 0 0 1 240 140"
              fill="none" stroke="#E6E9EF" stroke-width="14" stroke-linecap="round"/>
        <path id="gaugeFill" d="M 20 140 A 110 110 0 0 1 240 140"
              fill="none" stroke="#FF5E5E" stroke-width="14" stroke-linecap="round"
              stroke-dasharray="345.58" stroke-dashoffset="345.58" style="transition: stroke-dashoffset 0.6s ease, stroke 0.3s ease;"/>
        <text id="batteryPercent" x="130" y="130" text-anchor="middle" dominant-baseline="middle"
              font-size="28" font-weight="700" fill="#222">--%</text>
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
          <text id="nextFeedPercent" x="130" y="130" text-anchor="middle" dominant-baseline="middle"
                font-size="28" font-weight="600" fill="#1976D2">— год — хв</text>
        </svg>
      </div>
      <div class="next-feed-title">До наступного годування</div>
    </div>
  </div>
  <div id="lowBatteryAlert" class="low-battery-alert" role="status" aria-live="polite">
    <span class="low-battery-alert-icon" aria-hidden="true">!</span>
    <span id="lowBatteryAlertText">Низький рівень заряду</span>
  </div>
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
  <button type="button" id="btnSaveSpeed" onclick="saveSpeed()">Зберегти швидкість</button>
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
    <input id="feedRepeats" type="number" min="1" max="20" value="1" style="width:96px; min-width:96px;">
  </div>
  <button type="button" id="btnSaveRepeats" onclick="saveRepeats()" style="margin-top: 0;">Зберегти</button>
  <button type="button" id="btnFeedNow" onclick="feedNow()" style="margin-top: 12px; background: linear-gradient(45deg, #f44336, #d32f2f);">Годувати зараз</button>
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
      <div class="section-subtitle" id="nextFeedingInlineLabel">До наступного годування:</div>
    </div>
  </div>
  <div id="feedTimesContainer">
    <!-- Блоки будуть додаватися динамічно -->
  </div>
  <button type="button" class="add-btn" id="btnAddFeeding" onclick="addFeedTime()">+ Додати годування</button>
  <button type="button" id="btnSaveAllTimes" onclick="saveFeedTimes()" style="margin-top: 8px;">Зберегти всі часи</button>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
const I18N = {
  uk: {
    appSubtitle: 'Розумна годівниця',
    batteryTitle: 'Стан батареї',
    batteryVoltagePrefix: 'Напруга:',
    chargingNow: 'Заряджається',
    lowBatteryAlert: 'Низький рівень заряду',
    nextFeedTitle: 'До наступного годування',
    manualControlTitle: 'Ручне керування',
    manualControlSubtitle: 'Керуйте серво-приводом вручну',
    servoAngle: 'Кут серво:',
    servoSpeed: 'Швидкість серво:',
    saveSpeed: 'Зберегти швидкість',
    manualFeedTitle: 'Ручне годування',
    manualFeedSubtitle: 'Швидкий запуск циклу годування',
    repeats: 'Кількість повторів',
    save: 'Зберегти',
    feedNow: 'Годувати зараз',
    autoFeedTitle: 'Автоматичне годування',
    autoFeedSubtitle: 'Налаштуйте розклад годувань',
    nextFeedingInlineLabel: 'До наступного годування:',
    timePrefix: 'Час:',
    sleepPrefix: 'Сон через:',
    sleepNow: 'зараз',
    sleepPaused: 'пауза',
    sleepReasonPrefix: 'Причина:',
    sleepReasonReady: 'готово до сну',
    sleepReasonPowerSaveOff: 'режим енергозбереження вимкнено',
    sleepReasonApMode: 'увімкнено AP режим',
    sleepReasonFeeding: 'триває годування',
    sleepReasonWebActive: 'веб-інтерфейс активний',
    sleepReasonRecentActivity: 'нещодавня активність',
    sleepReasonDisplayAwake: 'дисплей ще активний',
    sleepReasonNextFeedSoon: 'наступне годування вже скоро',
    sleepReasonUnknown: 'стан уточнюється',
    addFeeding: '+ Додати годування',
    saveAllTimes: 'Зберегти всі часи',
    toastFeedBlocked: 'Занадто рано: мінімальний інтервал між годуваннями.',
    feedNowInProgress: 'Годую...',
    deepSleepHelpBanner: 'Якщо сторінка не відкривається — пристрій може бути в <strong>глибокому сні</strong>. Натисніть фізичну кнопку на корпусі, щоб прокинути та знову відкрити веб.',
    tabHome: 'Головна',
    tabInfo: 'Інформація',
    tabSettings: 'Налаштування',
    toastDefault: 'Збережено',
    toastSaveError: 'Помилка збереження',
    wifiScanning: 'Сканування мереж...',
    wifiNoNetworks: 'Мережі не знайдено',
    wifiSelect: 'Обрати',
    wifiScanDone: 'Сканування завершено',
    wifiScanError: 'Помилка сканування',
    wifiReconnecting: 'Перезапуск підключення...',
    wifiReconnectDone: 'Підключення перезапущено',
    wifiReconnectError: 'Помилка перезапуску',
    wifiEnterSsid: 'Введіть назву WiFi мережі',
    wifiSavedRestarting: 'WiFi збережено, перезапуск підключення...',
    wifiSaveError: 'Помилка збереження WiFi',
    feedBlockTime: 'Час:',
    feedBlockRepeats: 'Повторів:',
    feedRemoveTitle: 'Видалити',
    feedAdded: 'Годування додано',
    feedRemoved: 'Годування видалено',
    feedTimesSaved: 'Часи збережено',
    nextFeedDash: '— год — хв',
    toastAngleUpdated: 'Кут змінено',
    toastSpeedSaved: 'Швидкість збережено',
    toastFeedingNow: 'Годую',
    toastFeedError: 'Помилка годування',
    wifiStatusAp: 'Режим точки доступу (AP) - ',
    wifiStatusConnected: 'Підключено до: ',
    wifiNotConnected: 'Не підключено',
    wifiUnknown: 'невідомо',
    wifiNotSet: 'не налаштовано',
    heroFishAria: 'Стилізована рибка'
  },
  en: {
    appSubtitle: 'Smart feeder',
    batteryTitle: 'Battery status',
    batteryVoltagePrefix: 'Voltage:',
    chargingNow: 'Charging',
    lowBatteryAlert: 'Low battery level',
    nextFeedTitle: 'Until next feeding',
    manualControlTitle: 'Manual control',
    manualControlSubtitle: 'Control the servo manually',
    servoAngle: 'Servo angle:',
    servoSpeed: 'Servo speed:',
    saveSpeed: 'Save speed',
    manualFeedTitle: 'Manual feeding',
    manualFeedSubtitle: 'Quick feed cycle start',
    repeats: 'Repeats',
    save: 'Save',
    feedNow: 'Feed now',
    autoFeedTitle: 'Automatic feeding',
    autoFeedSubtitle: 'Set your feeding schedule',
    nextFeedingInlineLabel: 'Until next feeding:',
    timePrefix: 'Time:',
    sleepPrefix: 'Sleep in:',
    sleepNow: 'now',
    sleepPaused: 'paused',
    sleepReasonPrefix: 'Reason:',
    sleepReasonReady: 'ready to sleep',
    sleepReasonPowerSaveOff: 'power save is off',
    sleepReasonApMode: 'AP mode is active',
    sleepReasonFeeding: 'feeding in progress',
    sleepReasonWebActive: 'web interface is active',
    sleepReasonRecentActivity: 'recent activity',
    sleepReasonDisplayAwake: 'display is still awake',
    sleepReasonNextFeedSoon: 'next feeding is soon',
    sleepReasonUnknown: 'checking status',
    addFeeding: '+ Add feeding',
    saveAllTimes: 'Save all times',
    toastFeedBlocked: 'Too soon: minimum time between feeds.',
    feedNowInProgress: 'Feeding...',
    deepSleepHelpBanner: 'If this page won\'t load, the device may be in <strong>deep sleep</strong>. Press the physical button on the unit to wake it and use the web UI again.',
    tabHome: 'Home',
    tabInfo: 'Info',
    tabSettings: 'Settings',
    toastDefault: 'Saved',
    toastSaveError: 'Save error',
    wifiScanning: 'Scanning networks...',
    wifiNoNetworks: 'No networks found',
    wifiSelect: 'Select',
    wifiScanDone: 'Scan complete',
    wifiScanError: 'Scan failed',
    wifiReconnecting: 'Restarting connection...',
    wifiReconnectDone: 'Connection restarted',
    wifiReconnectError: 'Restart error',
    wifiEnterSsid: 'Enter WiFi network name',
    wifiSavedRestarting: 'WiFi saved, restarting connection...',
    wifiSaveError: 'WiFi save error',
    feedBlockTime: 'Time:',
    feedBlockRepeats: 'Repeats:',
    feedRemoveTitle: 'Remove',
    feedAdded: 'Feeding slot added',
    feedRemoved: 'Feeding slot removed',
    feedTimesSaved: 'Schedule saved',
    nextFeedDash: '— h — m',
    toastAngleUpdated: 'Angle updated',
    toastSpeedSaved: 'Speed saved',
    toastFeedingNow: 'Feeding now',
    toastFeedError: 'Feed error',
    wifiStatusAp: 'Access Point mode (AP) - ',
    wifiStatusConnected: 'Connected to: ',
    wifiNotConnected: 'Not connected',
    wifiUnknown: 'unknown',
    wifiNotSet: 'not set',
    heroFishAria: 'Stylized fish'
  }
};
let currentLang = getStoredUiLang();
function t(key) {
  return (I18N[currentLang] && I18N[currentLang][key]) || (I18N.uk[key] || key);
}


function applyLanguage() {
  const sections = document.querySelectorAll('.section-title');
  const subtitles = document.querySelectorAll('.section-subtitle');
  const labels = document.querySelectorAll('label');
  const tabs = document.querySelectorAll('.bottom-tab span');
  const appSubtitle = document.querySelector('.app-subtitle');
  const batteryTitle = document.querySelector('.battery-title');
  const batterySubtitle = document.querySelector('.battery-subtitle');
  const nextFeedTitle = document.querySelector('.next-feed-title');
  if (appSubtitle) appSubtitle.textContent = t('appSubtitle');
  if (batteryTitle) batteryTitle.textContent = t('batteryTitle');
  if (batterySubtitle) batterySubtitle.innerHTML = `${t('batteryVoltagePrefix')} <span id="batteryVoltage">--</span> V`;
  if (nextFeedTitle) nextFeedTitle.textContent = t('nextFeedTitle');
  const lowBatteryAlertText = document.getElementById('lowBatteryAlertText');
  if (lowBatteryAlertText) lowBatteryAlertText.textContent = t('lowBatteryAlert');
  const currentBatteryPercentLabel = document.getElementById('batteryPercent');
  const currentBatteryPercent = currentBatteryPercentLabel
    ? Number(String(currentBatteryPercentLabel.textContent || '').replace('%', '').trim())
    : NaN;
  updateLowBatteryAlert(Number.isFinite(currentBatteryPercent) ? currentBatteryPercent : null, batteryChargingNow);
  const deepSleepBanner = document.getElementById('deepSleepHelpBanner');
  if (deepSleepBanner) deepSleepBanner.innerHTML = t('deepSleepHelpBanner');
  if (sections[0]) sections[0].textContent = t('manualControlTitle');
  if (subtitles[0]) subtitles[0].textContent = t('manualControlSubtitle');
  if (sections[1]) sections[1].textContent = t('manualFeedTitle');
  if (subtitles[1]) subtitles[1].textContent = t('manualFeedSubtitle');
  if (sections[2]) sections[2].textContent = t('autoFeedTitle');
  if (subtitles[2]) subtitles[2].textContent = t('autoFeedSubtitle');
  const nextFeedingInlineLabel = document.getElementById('nextFeedingInlineLabel');
  if (nextFeedingInlineLabel) nextFeedingInlineLabel.textContent = `${t('nextFeedingInlineLabel')} ${t('nextFeedDash')}`;
  if (labels[0]) labels[0].innerHTML = `${t('servoAngle')} <span id="angleLabel">0</span>°`;
  if (labels[1]) labels[1].innerHTML = `${t('servoSpeed')} <span id="speedValue">20</span>`;
  const sleepCountdownLabel = document.getElementById('sleepCountdownLabel');
  const sleepReasonLabel = document.getElementById('sleepReasonLabel');
  if (sleepCountdownLabel && !sleepCountdownLabel.dataset.dynamic) sleepCountdownLabel.textContent = `${t('sleepPrefix')} --`;
  if (sleepReasonLabel && !sleepReasonLabel.dataset.dynamic) sleepReasonLabel.textContent = `${t('sleepReasonPrefix')} --`;
  const btnSaveSpeed = document.getElementById('btnSaveSpeed');
  const btnSaveRepeats = document.getElementById('btnSaveRepeats');
  const btnAddFeeding = document.getElementById('btnAddFeeding');
  const btnSaveAllTimes = document.getElementById('btnSaveAllTimes');
  if (btnSaveSpeed) btnSaveSpeed.textContent = t('saveSpeed');
  if (btnSaveRepeats) btnSaveRepeats.textContent = t('save');
  if (btnAddFeeding) btnAddFeeding.textContent = t('addFeeding');
  if (btnSaveAllTimes) btnSaveAllTimes.textContent = t('saveAllTimes');
  renderFeedNowButtonState();
  renderManualFeedHint(manualFeedCooldownLocal);
  const repeatsLabel = document.querySelector('.flex-row span');
  if (repeatsLabel) repeatsLabel.textContent = t('repeats');
  if (tabs[0]) tabs[0].textContent = t('tabHome');
  if (tabs[1]) tabs[1].textContent = t('tabInfo');
  if (tabs[2]) tabs[2].textContent = t('tabSettings');
  document.querySelectorAll('.feed-block .flex-row').forEach(row => {
    const spans = row.querySelectorAll('span');
    if (spans.length >= 2) {
      spans[0].textContent = t('feedBlockTime');
      spans[1].textContent = t('feedBlockRepeats');
    }
    const removeBtn = row.querySelector('.remove-btn');
    if (removeBtn) removeBtn.title = t('feedRemoveTitle');
  });
  const toastEl = document.getElementById('toast');
  if (toastEl) toastEl.textContent = t('toastDefault');
  const nextFeedPct = document.getElementById('nextFeedPercent');
  if (nextFeedPct) nextFeedPct.textContent = t('nextFeedDash');
  const heroFish = document.querySelector('.hero-svg-fish');
  if (heroFish) heroFish.setAttribute('aria-label', t('heroFishAria'));
}
function updateAngleLabel(v){ document.getElementById('angleLabel').innerText=v; }
function updateSpeed(v){ document.getElementById('speedValue').innerText=v; }

function formatSleepCountdown(seconds) {
  if (!Number.isFinite(seconds) || seconds < 0) {
    return t('sleepPaused');
  }
  if (seconds <= 0) {
    return t('sleepNow');
  }

  const mins = Math.floor(seconds / 60);
  const secs = seconds % 60;
  if (mins > 0) {
    return `${mins}m ${secs}s`;
  }
  return `${secs}s`;
}

function getSleepReasonText(reason) {
  switch (reason) {
    case 'ready': return t('sleepReasonReady');
    case 'power_save_off': return t('sleepReasonPowerSaveOff');
    case 'ap_mode': return t('sleepReasonApMode');
    case 'feeding': return t('sleepReasonFeeding');
    case 'web_active': return t('sleepReasonWebActive');
    case 'recent_activity': return t('sleepReasonRecentActivity');
    case 'display_awake': return t('sleepReasonDisplayAwake');
    case 'next_feed_soon': return t('sleepReasonNextFeedSoon');
    default: return t('sleepReasonUnknown');
  }
}

function formatManualFeedCountdown(totalSeconds) {
  const s = Math.max(0, Math.floor(Number(totalSeconds) || 0));
  const mins = Math.floor(s / 60);
  const secs = s % 60;
  if (currentLang === 'en') {
    if (mins > 0) return `${mins}m ${secs}s`;
    return `${secs}s`;
  }
  if (mins > 0) return `${mins} хв ${secs} с`;
  return `${secs} с`;
}

let manualFeedCooldownLocal = 0;
let manualFeedCooldownInterval = null;
let manualFeedInProgress = false;
let manualFeedAnimationTimer = null;

function renderFeedNowButtonState() {
  const btn = document.getElementById('btnFeedNow');
  if (!btn) return;
  const cooldownSec = Math.max(0, Math.floor(Number(manualFeedCooldownLocal) || 0));
  const inCooldown = cooldownSec > 0;
  const isDisabled = manualFeedInProgress || inCooldown;

  btn.disabled = isDisabled;
  btn.style.opacity = isDisabled ? '0.72' : '';
  btn.style.cursor = isDisabled ? 'not-allowed' : '';
  btn.classList.toggle('is-feeding', manualFeedInProgress);

  if (manualFeedInProgress) {
    btn.textContent = t('feedNowInProgress');
  } else if (inCooldown) {
    btn.textContent = `${t('feedNow')} (${formatManualFeedCountdown(cooldownSec)})`;
  } else {
    btn.textContent = t('feedNow');
  }
}

function renderManualFeedHint(seconds) {
  manualFeedCooldownLocal = Math.max(0, Math.floor(Number(seconds) || 0));
  renderFeedNowButtonState();
}

function syncManualFeedCooldownFromStatus(seconds) {
  renderManualFeedHint(seconds);
  if (manualFeedCooldownInterval) {
    clearInterval(manualFeedCooldownInterval);
    manualFeedCooldownInterval = null;
  }
  if (manualFeedCooldownLocal > 0) {
    manualFeedCooldownInterval = setInterval(() => {
      manualFeedCooldownLocal = Math.max(0, manualFeedCooldownLocal - 1);
      renderManualFeedHint(manualFeedCooldownLocal);
      if (manualFeedCooldownLocal <= 0 && manualFeedCooldownInterval) {
        clearInterval(manualFeedCooldownInterval);
        manualFeedCooldownInterval = null;
      }
    }, 1000);
  }
}

let angleUpdateTimeout = null;
let lastQueuedAngle = null;
let lastSentAngle = null;

function flushAngleUpdate(showNotification = false) {
  if (lastQueuedAngle === null || lastQueuedAngle === lastSentAngle) return;
  const angleToSend = lastQueuedAngle;
  postForm('/api/setAngle', { angle: angleToSend })
    .then(expectOk)
    .then(() => {
      lastSentAngle = angleToSend;
      if (showNotification) {
        showToast(t('toastAngleUpdated'));
      }
    })
    .catch(() => {
      if (showNotification) {
        showToast(t('toastSaveError'));
      }
    });
}

document.getElementById('angleSlider').addEventListener('input', function(){
  const val = this.value;
  updateAngleLabel(val);
  lastQueuedAngle = val;
  if (angleUpdateTimeout) {
    clearTimeout(angleUpdateTimeout);
  }
  angleUpdateTimeout = setTimeout(() => {
    angleUpdateTimeout = null;
    flushAngleUpdate(false);
  }, 150);
});

document.getElementById('angleSlider').addEventListener('change', function() {
  if (angleUpdateTimeout) {
    clearTimeout(angleUpdateTimeout);
    angleUpdateTimeout = null;
  }
  flushAngleUpdate(true);
});

function showToast(text = t('toastDefault')) {
  showToastMessage(text);
}

function showActionError(error, fallbackText = t('toastSaveError')) {
  showToast((error && error.message) ? error.message : fallbackText);
}

function feedNow(){
  if (manualFeedInProgress) return;
  if (manualFeedCooldownLocal > 0) {
    showToast(t('toastFeedBlocked'));
    return;
  }

  manualFeedInProgress = true;
  renderFeedNowButtonState();

  const repeatsEl = document.getElementById('feedRepeats');
  const repeats = repeatsEl ? repeatsEl.value : 1;
  postForm('/api/feedNow', { repeats })
    .then(async (response) => {
      if (response.ok) {
        showToast(t('toastFeedingNow'));
        statusUpdate();
        return;
      }

      const message = await response.text();
      if (response.status === 429) {
        showToast(t('toastFeedBlocked'));
        statusUpdate();
      } else {
        showToast(message || t('toastFeedError'));
      }
    })
    .catch(() => {
      showToast(t('toastFeedError'));
    })
    .finally(() => {
      if (manualFeedAnimationTimer) clearTimeout(manualFeedAnimationTimer);
      manualFeedAnimationTimer = setTimeout(() => {
        manualFeedInProgress = false;
        renderFeedNowButtonState();
      }, 1800);
    });
}
function saveSpeed(){ const s=document.getElementById('speedSlider').value; postForm('/api/setSpeed', { speed: s }).then(expectOk).then(()=>{statusUpdate(); showToast(t('toastSpeedSaved'));}).catch(error => showActionError(error)); }
function saveRepeats(){ const r=document.getElementById('feedRepeats').value; postForm('/api/setRepeats', { repeats: r }).then(expectOk).then(()=>{statusUpdate(); showToast(t('toastDefault'));}).catch(error => showActionError(error)); }
function scanWiFi(){
  showToast(t('wifiScanning'));
  fetch('/api/scanWiFi')
    .then(r=>r.json())
    .then(networks=>{
      const listDiv = document.getElementById('wifiList');
      const networksDiv = document.getElementById('wifiNetworks');
      if (!networksDiv || !listDiv) return;
      networksDiv.innerHTML = '';
      
      if(networks.length === 0) {
        networksDiv.innerHTML = '<div class="networks-empty">' + t('wifiNoNetworks') + '</div>';
      } else {
        networks.forEach(net => {
          const netDiv = document.createElement('div');
          netDiv.className = 'network-item';
          netDiv.onclick = function() {
            const ssidEl = document.getElementById('wifiSSID');
            if (ssidEl) ssidEl.value = net.ssid;
            const passEl = document.getElementById('wifiPassword');
            if (passEl) passEl.focus();
          };
          netDiv.innerHTML = `
            <div>
              <strong>${net.ssid}</strong>
              <span class="network-signal">${net.rssi} dBm</span>
              ${net.encrypted ? '<span class="network-lock">🔒</span>' : ''}
            </div>
            <span class="network-action">${t('wifiSelect')}</span>
          `;
          networksDiv.appendChild(netDiv);
        });
      }
      listDiv.style.display = 'block';
      showToast(t('wifiScanDone'));
    })
    .catch(()=>{
      showToast(t('wifiScanError'));
    });
}

function reconnectWiFi(){
  showToast(t('wifiReconnecting'));
  postForm('/api/reconnectWiFi')
    .then(expectOk)
    .then(()=>{
      showToast(t('wifiReconnectDone'));
      setTimeout(()=>{
        statusUpdate();
      }, 2000);
    })
    .catch(error => showActionError(error, t('wifiReconnectError')));
}

function saveWiFi(){ 
  const ssid = document.getElementById('wifiSSID');
  const password = document.getElementById('wifiPassword');
  if (!ssid || !password) return;
  if(!ssid.value || ssid.value.trim() === '') {
    showToast(t('wifiEnterSsid'));
    return;
  }
  postForm('/api/setWiFi', { ssid: ssid.value, password: password.value })
    .then(expectOk)
    .then(()=>{
      showToast(t('wifiSavedRestarting'));
      setTimeout(()=>{
        window.location.reload();
      }, 3000);
    })
    .catch(error => showActionError(error, t('wifiSaveError')));
}
let feedTimeCounter = 0;

function clampFeedValue(value, min, max, fallback) {
  const parsed = parseInt(value, 10);
  if (isNaN(parsed)) return fallback;
  return Math.min(max, Math.max(min, parsed));
}

function readFeedTimeField(item, shortKey, longKey, fallback) {
  if (item && item[shortKey] !== undefined && item[shortKey] !== null) return item[shortKey];
  if (item && item[longKey] !== undefined && item[longKey] !== null) return item[longKey];
  return fallback;
}

function formatFeedTimeValue(hour, minute) {
  return `${String(clampFeedValue(hour, 0, 23, 10)).padStart(2, '0')}:${String(clampFeedValue(minute, 0, 59, 0)).padStart(2, '0')}`;
}

function parseFeedTimeValue(value) {
  const match = /^(\d{1,2}):(\d{1,2})$/.exec(String(value || '').trim());
  if (!match) {
    return { hour: 10, minute: 0 };
  }
  return {
    hour: clampFeedValue(match[1], 0, 23, 10),
    minute: clampFeedValue(match[2], 0, 59, 0),
  };
}

function addFeedTime(hour = 10, minute = 0, repeats = 1, showNotification = true) {
  const container = document.getElementById('feedTimesContainer');
  if (!container) {
    console.error('feedTimesContainer not found');
    return;
  }
  const blockId = 'feedBlock_' + feedTimeCounter++;
  const block = document.createElement('div');
  block.className = 'feed-block';
  block.id = blockId;
  block.innerHTML = `
    <div class="flex-row">
      <span>${t('feedBlockTime')}</span>
      <input type="time" class="feed-time" step="60" value="${formatFeedTimeValue(hour, minute)}" style="padding: 4px;">
      <span>${t('feedBlockRepeats')}</span>
      <input type="number" class="feed-repeats" min="1" max="20" value="${clampFeedValue(repeats, 1, 20, 1)}">
      <button class="remove-btn" onclick="removeFeedTime('${blockId}')" title="${t('feedRemoveTitle')}">×</button>
    </div>
  `;
  container.appendChild(block);
  if (showNotification) {
    showToast(t('feedAdded'));
  }
}

function removeFeedTime(blockId) {
  const block = document.getElementById(blockId);
  if (block) {
    block.remove();
    showToast(t('feedRemoved'));
  }
}

function saveFeedTimes(){
  const blocks = document.querySelectorAll('.feed-block');
  const feedTimes = [];
  blocks.forEach(block => {
    const timeValue = block.querySelector('.feed-time').value;
    const { hour, minute } = parseFeedTimeValue(timeValue);
    const repeats = clampFeedValue(block.querySelector('.feed-repeats').value, 1, 20, 1);
    feedTimes.push({ hour, minute, repeats });
  });
  const data = JSON.stringify(feedTimes);
  postForm('/api/setFeedTimes', { data }).then(expectOk).then(()=>{statusUpdate(); showToast(t('feedTimesSaved'));}).catch(error => showActionError(error));
}


function loadFeedTimes(feedTimes) {
  const container = document.getElementById('feedTimesContainer');
  if (!container) return;
  container.innerHTML = '';
  feedTimeCounter = 0; // Скидаємо лічильник
  if (feedTimes && feedTimes.length > 0) {
    feedTimes.forEach(ft => {
      addFeedTime(
        readFeedTimeField(ft, 'h', 'hour', 10),
        readFeedTimeField(ft, 'm', 'minute', 0),
        readFeedTimeField(ft, 'r', 'repeats', 1),
        false
      );
    });
  } else {
    // Якщо немає збережених годувань, додаємо одне стандартне
    addFeedTime(10, 0, 1, false);
  }
}

function normalizeFeedSchedule(status) {
  const schedule = [];
  let feedArray = [];
  if (Array.isArray(status.feedTimes)) {
    feedArray = status.feedTimes;
  } else if (typeof status.feedTimes === 'string' && status.feedTimes.trim().length && status.feedTimes.trim() !== 'null') {
    try {
      const parsed = JSON.parse(status.feedTimes);
      if (Array.isArray(parsed)) {
        feedArray = parsed;
      }
    } catch (e) {
      console.warn('Unable to parse feedTimes string', e);
    }
  }
  if (feedArray.length) {
    feedArray.forEach(ft => {
      const rawHour = ft && ft.h !== undefined ? ft.h : (ft && ft.hour !== undefined ? ft.hour : 0);
      const rawMinute = ft && ft.m !== undefined ? ft.m : (ft && ft.minute !== undefined ? ft.minute : 0);
      const hour = parseInt(rawHour, 10);
      const minute = parseInt(rawMinute, 10);
      if (!isNaN(hour) && !isNaN(minute)) {
        const normHour = ((hour % 24) + 24) % 24;
        const normMinute = ((minute % 60) + 60) % 60;
        schedule.push({ hour: normHour, minute: normMinute, total: normHour * 60 + normMinute });
      }
    });
  }

  if (!schedule.length && status.feedHour1 !== undefined && status.feedMinute1 !== undefined) {
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
  if (minutes <= 0) return currentLang === 'en' ? '0 h 0 m' : '0 год 0 хв';
  const hours = Math.floor(minutes / 60);
  const mins = minutes % 60;
  return currentLang === 'en' ? `${hours} h ${mins} m` : `${hours} год ${mins} хв`;
}

let hasNextFeedGaugeIntroAnimated = false;

function updateNextFeedingProgress(status) {
  const percentTextEl = document.getElementById('nextFeedPercent');
  const fillPath = document.getElementById('nextFeedFill');
  const inlineLabelEl = document.getElementById('nextFeedingInlineLabel');
  if (!percentTextEl || !fillPath) return;

  const schedule = normalizeFeedSchedule(status);
  let minutesUntilNext = (typeof status.nextFeedMinutes === 'number' && status.nextFeedMinutes >= 0)
    ? status.nextFeedMinutes
    : null;
  const targetHour = (typeof status.nextFeedHour === 'number' && status.nextFeedHour >= 0)
    ? status.nextFeedHour
    : null;
  const targetMinute = (typeof status.nextFeedMinute === 'number' && status.nextFeedMinute >= 0)
    ? status.nextFeedMinute
    : null;

  if (!schedule.length) {
    if (minutesUntilNext !== null) {
      percentTextEl.textContent = formatDurationMinutes(minutesUntilNext);
    } else {
      percentTextEl.textContent = t('nextFeedDash');
    }
    fillPath.style.strokeDasharray = 345.58;
    fillPath.style.strokeDashoffset = 345.58;
    if (inlineLabelEl) {
      inlineLabelEl.textContent = `${t('nextFeedingInlineLabel')} ${percentTextEl.textContent}`;
    }
    return;
  }

  const now = new Date();
  const nowMinutes = now.getHours() * 60 + now.getMinutes();
  const circumference = 345.58;
  fillPath.style.strokeDasharray = circumference;

  let nextIndex = -1;
  if (targetHour !== null && targetMinute !== null) {
    const targetTotal = ((targetHour % 24) + 24) % 24 * 60 + (((targetMinute % 60) + 60) % 60);
    nextIndex = schedule.findIndex(item => item.total === targetTotal);
  }
  if (nextIndex === -1) {
    nextIndex = schedule.findIndex(item => item.total > nowMinutes);
    if (nextIndex === -1) nextIndex = 0;
  }

  const next = schedule[nextIndex];
  const prev = schedule[(nextIndex - 1 + schedule.length) % schedule.length];

  if (minutesUntilNext === null) {
    minutesUntilNext = next.total - nowMinutes;
    if (minutesUntilNext <= 0) minutesUntilNext += 24 * 60;
  }

  let interval = next.total - prev.total;
  if (interval <= 0) interval += 24 * 60;

  const clampedMinutes = Math.max(0, Math.min(minutesUntilNext, interval));
  const percent = interval > 0 ? Math.max(0, Math.min(100, (clampedMinutes / interval) * 100)) : 0;

  const offset = circumference - (percent / 100) * circumference;
  if (!hasNextFeedGaugeIntroAnimated) {
    fillPath.style.transition = 'none';
    fillPath.style.strokeDashoffset = circumference;
    void fillPath.getBoundingClientRect();
    fillPath.style.transition = 'stroke-dashoffset 1.15s cubic-bezier(0.22, 1, 0.36, 1), stroke 0.3s ease';
    requestAnimationFrame(() => {
      fillPath.style.strokeDashoffset = offset;
    });
    hasNextFeedGaugeIntroAnimated = true;
  } else {
    fillPath.style.transition = 'stroke-dashoffset 0.6s ease, stroke 0.3s ease';
    fillPath.style.strokeDashoffset = offset;
  }
  percentTextEl.textContent = formatDurationMinutes(minutesUntilNext);
  if (inlineLabelEl) {
    inlineLabelEl.textContent = `${t('nextFeedingInlineLabel')} ${percentTextEl.textContent}`;
  }
}

let hasBatteryGaugeIntroAnimated = false;

function updateBatteryGauge(percent) {
  const gaugeFill = document.getElementById('gaugeFill');
  const percentLabel = document.getElementById('batteryPercent');
  if (!gaugeFill || !percentLabel) return;

  if (!Number.isFinite(percent)) {
    const circumference = 345.58;
    gaugeFill.classList.remove('battery-gauge-charging');
    gaugeFill.style.transition = '';
    gaugeFill.style.strokeDasharray = circumference;
    gaugeFill.style.strokeDashoffset = circumference;
    percentLabel.textContent = '--%';
    percentLabel.setAttribute('fill', '#4CAF50');
    const gaugeWrapper = document.querySelector('.battery-gauge-wrapper');
    if (gaugeWrapper) {
      gaugeWrapper.removeAttribute('data-low-battery');
    }
    updateLowBatteryAlert(null, batteryChargingNow);
    return;
  }

  const safePercent = Math.max(0, Math.min(100, percent));
  const radius = 120;
  const circumference = Math.PI * radius;
  const offset = circumference - (safePercent / 100) * circumference;

  gaugeFill.style.strokeDasharray = circumference;

  let color = batteryChargingNow ? '#4CAF50' : (safePercent >= 75 ? '#4CAF50' : safePercent >= 35 ? '#FFB300' : '#D32F2F');
  gaugeFill.style.stroke = color;
  percentLabel.setAttribute('fill', color);
  percentLabel.textContent = `${safePercent}%`;

  if (batteryChargingNow) {
    gaugeFill.style.setProperty('--charge-full', circumference);
    gaugeFill.style.setProperty('--charge-target', offset);
    gaugeFill.classList.add('battery-gauge-charging');
    hasBatteryGaugeIntroAnimated = true;
  } else {
    gaugeFill.classList.remove('battery-gauge-charging');
    if (!hasBatteryGaugeIntroAnimated) {
      gaugeFill.style.transition = 'none';
      gaugeFill.style.strokeDashoffset = circumference;
      void gaugeFill.getBoundingClientRect();
      gaugeFill.style.transition = 'stroke-dashoffset 1.15s cubic-bezier(0.22, 1, 0.36, 1), stroke 0.3s ease';
      requestAnimationFrame(() => {
        gaugeFill.style.strokeDashoffset = offset;
      });
      hasBatteryGaugeIntroAnimated = true;
    } else {
      gaugeFill.style.transition = 'stroke-dashoffset 0.6s ease, stroke 0.3s ease';
      gaugeFill.style.strokeDashoffset = offset;
    }
  }

  const gaugeWrapper = document.querySelector('.battery-gauge-wrapper');
  if (gaugeWrapper) {
    if (safePercent <= 15) {
      gaugeWrapper.setAttribute('data-low-battery', 'true');
    } else {
      gaugeWrapper.removeAttribute('data-low-battery');
    }
  }
  updateLowBatteryAlert(safePercent, batteryChargingNow);
}

const LOW_BATTERY_ALERT_THRESHOLD = 20;
let batteryChargingNow = false;

function updateLowBatteryAlert(percent, isCharging = false) {
  const alertEl = document.getElementById('lowBatteryAlert');
  const textEl = document.getElementById('lowBatteryAlertText');
  const iconEl = alertEl ? alertEl.querySelector('.low-battery-alert-icon') : null;
  if (!alertEl) return;

  if (isCharging) {
    if (textEl) textEl.textContent = t('chargingNow');
    if (iconEl) iconEl.textContent = 'i';
    alertEl.classList.remove('is-low');
    alertEl.classList.add('is-charging');
    alertEl.classList.add('is-visible');
    return;
  }

  if (Number.isFinite(percent) && percent <= LOW_BATTERY_ALERT_THRESHOLD) {
    if (textEl) textEl.textContent = t('lowBatteryAlert');
    if (iconEl) iconEl.textContent = '!';
    alertEl.classList.remove('is-charging');
    alertEl.classList.add('is-low');
    alertEl.classList.add('is-visible');
  } else {
    alertEl.classList.remove('is-charging');
    alertEl.classList.remove('is-low');
    alertEl.classList.remove('is-visible');
  }
}

function statusUpdate(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    const batteryVoltageEl = document.getElementById('batteryVoltage');
    batteryChargingNow = !!j.isCharging;
    if (batteryVoltageEl) {
      if (typeof j.batteryVoltage === 'number' && Number.isFinite(j.batteryVoltage)) {
        batteryVoltageEl.innerText = j.batteryVoltage.toFixed(2);
      } else {
        batteryVoltageEl.innerText = '--';
      }
    }

    const timeLabel = document.getElementById('localTimeLabel');
    if (timeLabel) {
      if (typeof j.currentTime === 'string') {
        timeLabel.innerText = `${t('timePrefix')} ` + j.currentTime;
      } else {
        timeLabel.innerText = `${t('timePrefix')} --:--`;
      }
    }

    const sleepCountdownLabel = document.getElementById('sleepCountdownLabel');
    if (sleepCountdownLabel) {
      sleepCountdownLabel.dataset.dynamic = 'true';
      sleepCountdownLabel.innerText = `${t('sleepPrefix')} ${formatSleepCountdown(Number(j.sleepCountdownSeconds))}`;
    }

    const sleepReasonLabel = document.getElementById('sleepReasonLabel');
    if (sleepReasonLabel) {
      sleepReasonLabel.dataset.dynamic = 'true';
      sleepReasonLabel.innerText = `${t('sleepReasonPrefix')} ${getSleepReasonText(j.sleepReason)}`;
    }

    let batteryPercentValue = null;
    const rawPercent = Number(j.batteryPercent);
    if (Number.isFinite(rawPercent)) {
      batteryPercentValue = Math.round(rawPercent);
    } else if (typeof j.batteryVoltage === 'number' || typeof j.batteryVoltage === 'string') {
      const computed = voltageToPercentClient(Number(j.batteryVoltage));
      if (Number.isFinite(computed)) {
        batteryPercentValue = computed;
      }
    }
    if (batteryPercentValue !== null) {
      updateBatteryGauge(Math.max(0, Math.min(100, batteryPercentValue)));
    } else {
      updateBatteryGauge(null);
    }
    updateNextFeedingProgress(j);
    syncManualFeedCooldownFromStatus(j.manualFeedCooldownSeconds);
    document.getElementById('angleSlider').value=j.currentAngle; updateAngleLabel(j.currentAngle);
    lastSentAngle = String(j.currentAngle);
    lastQueuedAngle = String(j.currentAngle);
    document.getElementById('speedSlider').value=j.speed; updateSpeed(j.speed);
    document.getElementById('feedRepeats').value=j.feedRepeats;
    const wifiSSIDInput = document.getElementById('wifiSSID');
    if(wifiSSIDInput && j.wifiSSID) {
      wifiSSIDInput.value = j.wifiSSID;
    }
    // Оновлюємо статус WiFi
    const statusText = document.getElementById('wifiStatusText');
    if (statusText) {
      if(j.isAPMode) {
        statusText.innerText = t('wifiStatusAp') + (j.wifiSSID || t('wifiNotSet'));
        statusText.style.color = '#FF9800';
      } else if(j.wifiIP) {
        statusText.innerText = t('wifiStatusConnected') + (j.wifiSSID || t('wifiUnknown')) + ' (IP: ' + j.wifiIP + ')';
        statusText.style.color = '#4CAF50';
      } else {
        statusText.innerText = t('wifiNotConnected');
        statusText.style.color = '#f44336';
      }
    }
    
    // Завантажуємо динамічні годування
    if (j.feedTimes && Array.isArray(j.feedTimes) && j.feedTimes.length > 0) {
      loadFeedTimes(j.feedTimes);
    } else if (j.feedHour1 !== undefined) {
      // Сумісність зі старим форматом
      loadFeedTimes([
        {h: j.feedHour1, m: j.feedMinute1, r: j.feedRepeats1 || 1},
        {h: j.feedHour2, m: j.feedMinute2, r: j.feedRepeats2 || 1}
      ]);
    } else {
      // Якщо немає збережених годувань, додаємо одне стандартне
      loadFeedTimes([]);
    }
  });
}
window.onload=function(){
  applyLanguage();
  statusUpdate();
  setActiveBottomTab({ matchRootToHome: true });
};

const lottieSrc = `{"v":"5.7.6","fr":30,"ip":0,"op":180,"w":400,"h":400,"nm":"Fish Jumping","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"Water","sr":1,"ks":{"o":{"a":0,"k":60},"r":{"a":0,"k":0},"p":{"a":0,"k":[200,320,0]},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,30,100]}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"el","p":{"a":0,"k":[0,0]},"s":{"a":0,"k":[280,120]},"nm":"Ellipse"},{"ty":"fl","c":{"a":0,"k":[0.196,0.545,0.765,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Water Base","hd":false}]} ,{"ddd":0,"ind":2,"ty":4,"nm":"Fish","sr":1,"ks":{"o":{"a":0,"k":100},"r":{"a":1,"k":[{"t":0,"s":[0],"e":[8],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]},{"t":90,"s":[8],"e":[-6],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]},{"t":180,"s":[-6],"e":[0],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]}]},"p":{"a":1,"k":[{"t":0,"s":[200,260,0],"e":[200,140,0],"i":{"x":[0.667,0.667],"y":[1,1]},"o":{"x":[0.333,0.333],"y":[0,0]},"to":[0,-20,0],"ti":[0,20,0]},{"t":90,"s":[200,140,0],"e":[200,260,0],"i":{"x":[0.667,0.667],"y":[1,1]},"o":{"x":[0.333,0.333],"y":[0,0]},"to":[0,20,0],"ti":[0,-20,0]},{"t":180}]},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,100,100]}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[220,110]},"p":{"a":0,"k":[0,0]},"r":{"a":0,"k":55},"nm":"body"},{"ty":"fl","c":{"a":0,"k":[0.988,0.596,0.349,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Fish Body","hd":false},{"ty":"gr","it":[{"ty":"el","p":{"a":0,"k":[90,-10]},"s":{"a":0,"k":[60,60]},"nm":"Eye"},{"ty":"fl","c":{"a":0,"k":[1,1,1,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"el","p":{"a":0,"k":[96,-10]},"s":{"a":0,"k":[28,28]},"nm":"Pupil"},{"ty":"fl","c":{"a":0,"k":[0.098,0.098,0.098,1]},"o":{"a":0,"k":100},"nm":"Fill 2"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Eye Group","hd":false},{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[120,80]},"p":{"a":0,"k":[110,0]},"r":{"a":0,"k":40},"nm":"Tail"},{"ty":"fl","c":{"a":0,"k":[0.961,0.471,0.373,1]},"o":{"a":0,"k":100},"nm":"Tail Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Tail","hd":false}]}]}`;

document.addEventListener('DOMContentLoaded', () => {
  const heroPlayer = document.getElementById('heroLottie');
  if (heroPlayer) {
    heroPlayer.load(lottieSrc);
  }
});
</script>

)rawliteral"
#include "shared/bottom_tabs_home.inc"
R"rawliteral(
</body>
</html>
)rawliteral";
