#include "web_pages.h"

// ============================================================================
// FRONTEND - Веб-сторінки (HTML/CSS/JavaScript)
// ============================================================================
// Цей файл містить HTML/CSS/JavaScript код для веб-інтерфейсу
// Розділення фронтенду від бекенду для кращої організації коду
// ============================================================================

// Головна сторінка (AquaFeed Control)
const char* pageIndex = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>AquaFeed Control</title>
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
.flex-row input, .flex-row select {width: auto; min-width: 20px; font-size: 12px;}
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
.bottom-tab.active {
  color: #111827;
  background: rgba(15, 23, 42, 0.1);
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
  stroke-width: 1.9;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.bottom-tab-icon-svg .filled {
  fill: currentColor;
  stroke: none;
}
.bottom-tab.active .bottom-tab-icon,
.bottom-tab.active .home-icon {
  color: #111827;
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
</style>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-annotation@1.3.1"></script>
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
    <input id="feedRepeats" type="number" min="1" max="20" value="1">
  </div>
  <button type="button" id="btnSaveRepeats" onclick="saveRepeats()" style="margin-top: 0;">Зберегти</button>
  <div id="manualFeedHint" style="margin-top: 10px; line-height: 1.35; font-size: 13px;"></div>
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
      <div class="section-subtitle" id="localTimeLabel">Час: --:--</div>
      <div class="section-subtitle" id="sleepCountdownLabel">Сон: --</div>
      <div class="section-subtitle" id="sleepReasonLabel">Причина: --</div>
    </div>
  </div>
  <div id="feedTimesContainer">
    <!-- Блоки будуть додаватися динамічно -->
  </div>
  <button type="button" class="add-btn" id="btnAddFeeding" onclick="addFeedTime()">+ Додати годування</button>
  <button type="button" id="btnSaveAllTimes" onclick="saveFeedTimes()" style="margin-top: 8px;">Зберегти всі часи</button>
</div>

<script>
const I18N = {
  uk: {
    appSubtitle: 'Розумна годівниця',
    batteryTitle: 'Стан батареї',
    batteryVoltagePrefix: 'Напруга:',
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
    sleepReasonUnknown: 'стан уточнюється',
    addFeeding: '+ Додати годування',
    saveAllTimes: 'Зберегти всі часи',
    manualFeedReadyHint: 'Можна запустити ручне годування.',
    manualFeedCooldownHint: 'Мінімальний інтервал між годуваннями — 5 хв. Наступне ручне годування — через %s.',
    toastFeedBlocked: 'Занадто рано: мінімальний інтервал між годуваннями.',
    deepSleepHelpBanner: 'Якщо сторінка не відкривається — пристрій може бути в <strong>глибокому сні</strong>. Натисніть фізичну кнопку на корпусі, щоб прокинути та знову відкрити веб.',
    tabHome: 'Головна',
    tabInfo: 'Інформація',
    tabSettings: 'Налаштування'
  },
  en: {
    appSubtitle: 'Smart feeder',
    batteryTitle: 'Battery status',
    batteryVoltagePrefix: 'Voltage:',
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
    sleepReasonUnknown: 'checking status',
    addFeeding: '+ Add feeding',
    saveAllTimes: 'Save all times',
    manualFeedReadyHint: 'You can start a manual feed.',
    manualFeedCooldownHint: 'Minimum 5 minutes between feeds. Next manual feed in %s.',
    toastFeedBlocked: 'Too soon: minimum time between feeds.',
    deepSleepHelpBanner: 'If this page won\'t load, the device may be in <strong>deep sleep</strong>. Press the physical button on the unit to wake it and use the web UI again.',
    tabHome: 'Home',
    tabInfo: 'Info',
    tabSettings: 'Settings'
  }
};
let currentLang = localStorage.getItem('aqua_lang') || 'uk';
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
  const deepSleepBanner = document.getElementById('deepSleepHelpBanner');
  if (deepSleepBanner) deepSleepBanner.innerHTML = t('deepSleepHelpBanner');
  if (sections[0]) sections[0].textContent = t('manualControlTitle');
  if (subtitles[0]) subtitles[0].textContent = t('manualControlSubtitle');
  if (sections[1]) sections[1].textContent = t('manualFeedTitle');
  if (subtitles[1]) subtitles[1].textContent = t('manualFeedSubtitle');
  if (sections[2]) sections[2].textContent = t('autoFeedTitle');
  if (subtitles[2]) subtitles[2].textContent = t('autoFeedSubtitle');
  if (labels[0]) labels[0].innerHTML = `${t('servoAngle')} <span id="angleLabel">0</span>°`;
  if (labels[1]) labels[1].innerHTML = `${t('servoSpeed')} <span id="speedValue">20</span>`;
  const sleepCountdownLabel = document.getElementById('sleepCountdownLabel');
  const sleepReasonLabel = document.getElementById('sleepReasonLabel');
  if (sleepCountdownLabel && !sleepCountdownLabel.dataset.dynamic) sleepCountdownLabel.textContent = `${t('sleepPrefix')} --`;
  if (sleepReasonLabel && !sleepReasonLabel.dataset.dynamic) sleepReasonLabel.textContent = `${t('sleepReasonPrefix')} --`;
  const btnSaveSpeed = document.getElementById('btnSaveSpeed');
  const btnSaveRepeats = document.getElementById('btnSaveRepeats');
  const btnFeedNow = document.getElementById('btnFeedNow');
  const btnAddFeeding = document.getElementById('btnAddFeeding');
  const btnSaveAllTimes = document.getElementById('btnSaveAllTimes');
  if (btnSaveSpeed) btnSaveSpeed.textContent = t('saveSpeed');
  if (btnSaveRepeats) btnSaveRepeats.textContent = t('save');
  if (btnFeedNow) btnFeedNow.textContent = t('feedNow');
  if (btnAddFeeding) btnAddFeeding.textContent = t('addFeeding');
  if (btnSaveAllTimes) btnSaveAllTimes.textContent = t('saveAllTimes');
  renderManualFeedHint(manualFeedCooldownLocal);
  const repeatsLabel = document.querySelector('.flex-row span');
  if (repeatsLabel) repeatsLabel.textContent = t('repeats');
  if (tabs[0]) tabs[0].textContent = t('tabHome');
  if (tabs[1]) tabs[1].textContent = t('tabInfo');
  if (tabs[2]) tabs[2].textContent = t('tabSettings');
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

function renderManualFeedHint(seconds) {
  const hint = document.getElementById('manualFeedHint');
  const btn = document.getElementById('btnFeedNow');
  if (!hint) return;
  const s = Math.max(0, Math.floor(Number(seconds) || 0));
  if (s <= 0) {
    hint.textContent = t('manualFeedReadyHint');
    hint.style.color = '';
    if (btn) { btn.disabled = false; btn.style.opacity = ''; btn.style.cursor = ''; }
    return;
  }
  hint.textContent = t('manualFeedCooldownHint').replace('%s', formatManualFeedCountdown(s));
  hint.style.color = '#6b7280';
  if (btn) { btn.disabled = true; btn.style.opacity = '0.65'; btn.style.cursor = 'not-allowed'; }
}

function syncManualFeedCooldownFromStatus(seconds) {
  manualFeedCooldownLocal = Math.max(0, Math.floor(Number(seconds) || 0));
  renderManualFeedHint(manualFeedCooldownLocal);
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

document.getElementById('angleSlider').addEventListener('input', function(){
  const val = this.value;
  updateAngleLabel(val);
  fetch('/api/setAngle?angle='+val).then(()=>{showToast(currentLang === 'en' ? 'Angle updated' : 'Кут змінено');});
});

function voltageToPercentClient(v) {
  const MAX_VOLTAGE = 8.4;
  const MIN_VOLTAGE = 6.6;
  if (!Number.isFinite(v)) return null;
  if (v >= MAX_VOLTAGE) return 100;
  if (v <= MIN_VOLTAGE) return 0;
  return Math.round(((v - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100);
}

function showToast(text = (currentLang === 'en' ? 'Saved' : 'Збережено')) {
  const toast = document.getElementById('toast');
  toast.innerText = text;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2000);
}

function feedNow(){
  fetch('/api/feedNow')
    .then(async (response) => {
      if (response.ok) {
        showToast(currentLang === 'en' ? 'Feeding now' : 'Годую');
        statusUpdate();
        return;
      }

      const message = await response.text();
      if (response.status === 429) {
        showToast(t('toastFeedBlocked'));
        statusUpdate();
      } else {
        showToast(message || (currentLang === 'en' ? 'Feed error' : 'Помилка годування'));
      }
    })
    .catch(() => {
      showToast(currentLang === 'en' ? 'Feed error' : 'Помилка годування');
    });
}
function saveSpeed(){ const s=document.getElementById('speedSlider').value; fetch('/api/setSpeed?speed='+s).then(()=>{statusUpdate(); showToast(currentLang === 'en' ? 'Speed saved' : 'Швидкість збережено');}); }
function saveRepeats(){ const r=document.getElementById('feedRepeats').value; fetch('/api/setRepeats?repeats='+r).then(()=>{statusUpdate(); showToast(currentLang === 'en' ? 'Saved' : 'Збережено');}); }
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
      <span>Час:</span>
      <input type="number" class="feed-hour" min="0" max="23" value="${hour}" style="width:30px; min-width:20px; padding: 4px;">
      <span>:</span>
      <input type="number" class="feed-minute" min="0" max="59" value="${minute}" style="width:30px; min-width:20px; padding: 4px;">
      <span>Повторів:</span>
      <input type="number" class="feed-repeats" min="1" max="20" value="${repeats}" style="width:30px; min-width:20px; padding: 4px;">
      <button class="remove-btn" onclick="removeFeedTime('${blockId}')" title="Видалити">×</button>
    </div>
  `;
  container.appendChild(block);
  if (showNotification) {
    showToast('Годування додано');
  }
}

function removeFeedTime(blockId) {
  const block = document.getElementById(blockId);
  if (block) {
    block.remove();
    showToast('Годування видалено');
  }
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
  fetch('/api/setFeedTimes?data=' + encodeURIComponent(data)).then(()=>{statusUpdate(); showToast('Часи збережено');});
}


function loadFeedTimes(feedTimes) {
  const container = document.getElementById('feedTimesContainer');
  if (!container) return;
  container.innerHTML = '';
  feedTimeCounter = 0; // Скидаємо лічильник
  if (feedTimes && feedTimes.length > 0) {
    feedTimes.forEach(ft => {
      addFeedTime(ft.h || ft.hour || 10, ft.m || ft.minute || 0, ft.r || ft.repeats || 1, false);
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

function updateNextFeedingProgress(status) {
  const percentTextEl = document.getElementById('nextFeedPercent');
  const fillPath = document.getElementById('nextFeedFill');
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
      percentTextEl.textContent = '— год — хв';
    }
    fillPath.style.strokeDasharray = 345.58;
    fillPath.style.strokeDashoffset = 345.58;
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
  fillPath.style.strokeDashoffset = offset;
  percentTextEl.textContent = formatDurationMinutes(minutesUntilNext);
}

function updateBatteryGauge(percent) {
  const gaugeFill = document.getElementById('gaugeFill');
  const percentLabel = document.getElementById('batteryPercent');
  if (!gaugeFill || !percentLabel) return;

  if (!Number.isFinite(percent)) {
    const circumference = 345.58;
    gaugeFill.style.strokeDasharray = circumference;
    gaugeFill.style.strokeDashoffset = circumference;
    percentLabel.textContent = '--%';
    percentLabel.setAttribute('fill', '#4CAF50');
    const gaugeWrapper = document.querySelector('.battery-gauge-wrapper');
    if (gaugeWrapper) {
      gaugeWrapper.removeAttribute('data-low-battery');
    }
    return;
  }

  const safePercent = Math.max(0, Math.min(100, percent));
  const radius = 120;
  const circumference = Math.PI * radius;
  const offset = circumference - (safePercent / 100) * circumference;

  gaugeFill.style.strokeDasharray = circumference;
  gaugeFill.style.strokeDashoffset = offset;
  percentLabel.textContent = `${safePercent}%`;

  let color = '#FF5E5E';
  if (safePercent >= 75) {
    color = '#4CAF50';
  } else if (safePercent >= 35) {
    color = '#FF9800';
  } else {
    color = '#D32F2F';
  }

  gaugeFill.style.stroke = color;
  percentLabel.setAttribute('fill', color);

  const gaugeWrapper = document.querySelector('.battery-gauge-wrapper');
  if (gaugeWrapper) {
    if (safePercent <= 15) {
      gaugeWrapper.setAttribute('data-low-battery', 'true');
    } else {
      gaugeWrapper.removeAttribute('data-low-battery');
    }
  }
}

function statusUpdate(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    const batteryVoltageEl = document.getElementById('batteryVoltage');
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
    if (typeof j.batteryVoltage === 'number' || typeof j.batteryVoltage === 'string') {
      const computed = voltageToPercentClient(Number(j.batteryVoltage));
      if (Number.isFinite(computed)) {
        batteryPercentValue = computed;
      }
    }
    if (batteryPercentValue === null) {
      const rawPercent = Number(j.batteryPercent);
      if (Number.isFinite(rawPercent)) {
        batteryPercentValue = Math.round(rawPercent);
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
        statusText.innerText = (currentLang === 'en' ? 'Access Point mode (AP) - ' : 'Режим точки доступу (AP) - ') + (j.wifiSSID || (currentLang === 'en' ? 'not set' : 'не налаштовано'));
        statusText.style.color = '#FF9800';
      } else if(j.wifiIP) {
        statusText.innerText = (currentLang === 'en' ? 'Connected to: ' : 'Підключено до: ') + (j.wifiSSID || (currentLang === 'en' ? 'unknown' : 'невідомо')) + ' (IP: ' + j.wifiIP + ')';
        statusText.style.color = '#4CAF50';
      } else {
        statusText.innerText = currentLang === 'en' ? 'Not connected' : 'Не підключено';
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
setInterval(statusUpdate,30000); // зменшено з 5 до 30 секунд
window.onload=function(){
  applyLanguage();
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

const lottieSrc = `{"v":"5.7.6","fr":30,"ip":0,"op":180,"w":400,"h":400,"nm":"Fish Jumping","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"Water","sr":1,"ks":{"o":{"a":0,"k":60},"r":{"a":0,"k":0},"p":{"a":0,"k":[200,320,0]},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,30,100]}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"el","p":{"a":0,"k":[0,0]},"s":{"a":0,"k":[280,120]},"nm":"Ellipse"},{"ty":"fl","c":{"a":0,"k":[0.196,0.545,0.765,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Water Base","hd":false}]} ,{"ddd":0,"ind":2,"ty":4,"nm":"Fish","sr":1,"ks":{"o":{"a":0,"k":100},"r":{"a":1,"k":[{"t":0,"s":[0],"e":[8],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]},{"t":90,"s":[8],"e":[-6],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]},{"t":180,"s":[-6],"e":[0],"i":{"x":[0.667],"y":[1]},"o":{"x":[0.333],"y":[0]},"to":[0],"ti":[0]}]},"p":{"a":1,"k":[{"t":0,"s":[200,260,0],"e":[200,140,0],"i":{"x":[0.667,0.667],"y":[1,1]},"o":{"x":[0.333,0.333],"y":[0,0]},"to":[0,-20,0],"ti":[0,20,0]},{"t":90,"s":[200,140,0],"e":[200,260,0],"i":{"x":[0.667,0.667],"y":[1,1]},"o":{"x":[0.333,0.333],"y":[0,0]},"to":[0,20,0],"ti":[0,-20,0]},{"t":180}]},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,100,100]}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[220,110]},"p":{"a":0,"k":[0,0]},"r":{"a":0,"k":55},"nm":"body"},{"ty":"fl","c":{"a":0,"k":[0.988,0.596,0.349,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Fish Body","hd":false},{"ty":"gr","it":[{"ty":"el","p":{"a":0,"k":[90,-10]},"s":{"a":0,"k":[60,60]},"nm":"Eye"},{"ty":"fl","c":{"a":0,"k":[1,1,1,1]},"o":{"a":0,"k":100},"nm":"Fill"},{"ty":"el","p":{"a":0,"k":[96,-10]},"s":{"a":0,"k":[28,28]},"nm":"Pupil"},{"ty":"fl","c":{"a":0,"k":[0.098,0.098,0.098,1]},"o":{"a":0,"k":100},"nm":"Fill 2"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Eye Group","hd":false},{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[120,80]},"p":{"a":0,"k":[110,0]},"r":{"a":0,"k":40},"nm":"Tail"},{"ty":"fl","c":{"a":0,"k":[0.961,0.471,0.373,1]},"o":{"a":0,"k":100},"nm":"Tail Fill"},{"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}],"nm":"Tail","hd":false}]}]}`;

document.addEventListener('DOMContentLoaded', () => {
  const heroPlayer = document.getElementById('heroLottie');
  if (heroPlayer) {
    heroPlayer.load(lottieSrc);
  }
});
</script>

<div class="bottom-tabs">
  <a href="/" class="bottom-tab active">
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
  <a href="/wifi" class="bottom-tab">
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

// Сторінка інформації про систему
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
.bottom-tab.active {
  color: #111827;
  background: rgba(15, 23, 42, 0.1);
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
  stroke-width: 1.9;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.bottom-tab-icon-svg .filled {
  fill: currentColor;
  stroke: none;
}
.bottom-tab.active .bottom-tab-icon,
.bottom-tab.active .home-icon {
  color: #111827;
}
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
    <span class="info-value">1.0</span>
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
function infoIsEn() { return (localStorage.getItem('aqua_lang') || 'uk') === 'en'; }
function applyInfoLanguage() {
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
}
function updateInfo(){
  fetch('/api/status').then(r=>r.json()).then(j=>{
    document.getElementById('infoSSID').innerText = j.wifiSSID || (infoIsEn() ? 'not set' : 'не налаштовано');
    document.getElementById('infoIP').innerText = j.wifiIP || (infoIsEn() ? 'not connected' : 'не підключено');
    document.getElementById('infoMode').innerText = j.isAPMode ? (infoIsEn() ? 'Access Point (AP)' : 'Точка доступу (AP)') : (infoIsEn() ? 'Station (STA)' : 'Станція (STA)');
    if (typeof j.batteryVoltage === 'number' && Number.isFinite(j.batteryVoltage)) {
      document.getElementById('infoVoltage').innerText = j.batteryVoltage.toFixed(2) + ' В';
    } else {
      document.getElementById('infoVoltage').innerText = '-- В';
    }
    const infoPercentEl = document.getElementById('infoPercent');
    let infoPercentVal = voltageToPercentClient(Number(j.batteryVoltage));
    if (!Number.isFinite(infoPercentVal)) {
      infoPercentVal = Number(j.batteryPercent);
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
  const toast = document.getElementById('toast');
  if (toast) {
    toast.innerText = text;
    toast.classList.add('show');
    setTimeout(() => {
      toast.classList.remove('show');
    }, 2000);
  }
}

window.onload = function() {
  applyInfoLanguage();
  updateInfo();
  setInterval(updateInfo, 10000);
  
  // Встановлюємо активну вкладку
  const currentPath = window.location.pathname;
  const tabs = document.querySelectorAll('.bottom-tab');
  tabs.forEach(tab => {
    tab.classList.remove('active');
    if (tab.getAttribute('href') === currentPath) {
      tab.classList.add('active');
    }
  });
};
</script>

<div class="bottom-tabs">
  <a href="/" class="bottom-tab">
    <svg class="bottom-tab-icon home-icon" width="22" height="22" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M10 20V14H14V20H19V12H22L12 3L2 12H5V20H10Z" fill="currentColor"/>
    </svg>
    <span>Головна</span>
  </a>
  <a href="/info" class="bottom-tab active">
    <svg class="bottom-tab-icon bottom-tab-icon-svg" viewBox="0 0 24 24">
      <circle cx="12" cy="12" r="10"/>
      <path d="M12 16v-4"/>
      <path d="M12 8h.01"/>
    </svg>
    <span>Інформація</span>
  </a>
  <a href="/wifi" class="bottom-tab">
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

