// Home page ("/") — always served by the ESP32 itself, independent of the
// backend. Mirrors the React SPA dashboard's visual language (gauge SVGs,
// schedule editor) so the device-local fallback UI feels consistent with
// the "real" app at frontend/src/pages/DeviceDashboardPage.tsx.
const char* pageHome = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>AquaFeed</title>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<style>
)rawliteral"
#include "shared/common_body_base_styles.inc"
R"rawliteral(
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
button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}
button:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 14px 22px rgba(0,0,0,0.2);
}
button:active:not(:disabled) {
  transform: translateY(0);
  box-shadow: 0 6px 14px rgba(0,0,0,0.16);
}
)rawliteral"
#include "shared/common_card_styles.inc"
R"rawliteral(
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
.toast-text { display: inline-block; }
.toast.show { opacity: 1; transform: translateY(0) scale(1); }
)rawliteral"
#include "shared/common_section_styles.inc"
R"rawliteral(
.section-header { margin-bottom: 16px; }
)rawliteral"
#include "shared/common_hero_styles.inc"
R"rawliteral(
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
  width: auto;
  flex: 0 0 auto;
  margin-top: 0;
}
.lang-btn.active { background: #111; color: #fff; }
.status-pill.success { background: rgba(76,175,80,0.14); color: #2e7d32; }
.status-pill.warning { background: rgba(255,152,0,0.14); color: #f57c00; }
.status-pill.error { background: rgba(244,67,54,0.14); color: #c62828; }
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
.note-text {
  font-size: 12px;
  color: #666;
  margin-top: 12px;
  line-height: 1.5;
}
.slider-row { margin-bottom: 4px; }
.slider-row label {
  display: flex;
  justify-content: space-between;
  font-size: 13px;
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 6px;
}
input[type=range] { width: 100%; margin: 4px 0; }

/* ── Gauges (mirrors frontend/src/App.css .aq-gauge-*) ── */
.aq-gauges-row { display: flex; justify-content: center; gap: 8px; align-items: flex-start; }
.aq-gauge-wrap {
  flex: 1 1 0;
  min-width: 0;
  max-width: 160px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
}
.aq-gauge-svg { width: 100%; height: auto; }
.aq-gauge-fill-arc { transition: stroke-dashoffset 0.6s ease, stroke 0.3s ease; }
@keyframes aq-gauge-charge {
  0%   { stroke-dashoffset: var(--charge-full, 345.58); }
  62%  { stroke-dashoffset: var(--charge-target, 0); }
  80%  { stroke-dashoffset: var(--charge-target, 0); }
  100% { stroke-dashoffset: var(--charge-full, 345.58); }
}
.aq-gauge-charging-arc { animation: aq-gauge-charge 2.4s cubic-bezier(0.4, 0, 0.2, 1) infinite; transition: none !important; }
.aq-gauge-title { font-size: 14px; font-weight: 600; color: #222; text-align: center; }
.aq-gauge-sub   { font-size: 13px; color: #888; text-align: center; }

/* ── Schedule editor (mirrors frontend/src/App.css .aq-schedule-* / .aq-feed-*) ── */
.aq-schedule-list { display: flex; flex-direction: column; gap: 8px; margin-bottom: 10px; }
.aq-feed-block {
  display: flex;
  flex-wrap: wrap;
  gap: 6px 12px;
  align-items: center;
  padding: 10px 10px;
  background: rgba(0,0,0,0.03);
  border-radius: 8px;
  border-left: 3px solid #8e24aa;
}
.aq-feed-time-col { display: flex; align-items: center; flex-shrink: 0; }
.aq-feed-center { display: flex; flex-direction: column; gap: 4px; align-items: flex-start; flex: 1; min-width: 140px; }
.aq-feed-field { display: inline-flex; align-items: center; gap: 6px; }
.aq-feed-field > span { font-size: 12px; color: #6b7280; }
.aq-time-custom { display: flex; align-items: center; gap: 2px; }
.aq-time-part {
  width: 46px;
  padding: 4px 2px;
  border-radius: 6px;
  text-align: center;
  appearance: none;
  -webkit-appearance: none;
  cursor: pointer;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 13px;
  font-weight: 700;
  color: #1f2937;
  font-family: inherit;
}
.aq-time-part:focus { outline: none; border-color: #1976D2; box-shadow: 0 0 0 2px rgba(25,118,210,0.12); }
.aq-time-sep { font-size: 15px; font-weight: 700; color: #374151; line-height: 1; }
.aq-day-select {
  width: 88px;
  padding: 3px 4px;
  border-radius: 6px;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 12px;
  color: #374151;
  font-family: inherit;
  cursor: pointer;
}
.aq-day-select:focus { outline: none; border-color: #1976D2; }
.aq-remove-btn {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  border: none;
  background: #f44336;
  color: #fff;
  font-size: 16px;
  font-weight: 700;
  line-height: 1;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 0;
  margin-top: 0;
  margin-left: auto;
  transition: background 0.15s;
  flex-shrink: 0;
}
.aq-remove-btn:hover { background: #d32f2f; }
.aq-add-btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: auto;
  padding: 10px 20px;
  margin: 4px 0;
  border: 1px solid rgba(15,23,42,0.12);
  border-radius: 999px;
  background: #fff;
  color: #333;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  font-family: inherit;
  transition: background 0.15s;
}
.aq-add-btn:hover { background: rgba(15,23,42,0.05); }
.aq-save-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  margin-top: 8px;
  padding: 12px 24px;
  border: none;
  border-radius: 999px;
  background: #111;
  color: #fff;
  font-size: 14px;
  font-weight: 700;
  cursor: pointer;
}
.aq-stepper { display: inline-flex; align-items: center; gap: 8px; }
.aq-stepper-btn {
  width: 28px;
  height: 28px;
  border-radius: 50%;
  border: 1px solid #d1d5db;
  background: #fff;
  color: #111827;
  font-size: 14px;
  font-weight: 700;
  line-height: 1;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 0;
  margin-top: 0;
  transition: background 0.15s;
}
.aq-stepper-btn:active { transform: scale(0.94); }
.aq-stepper-btn:disabled { opacity: 0.4; cursor: not-allowed; }
.aq-stepper-val { min-width: 24px; text-align: center; font-size: 14px; font-weight: 700; color: #111827; }

/* ── Feed button (mirrors .aq-feed-btn) ── */
.aq-feed-btn {
  position: relative;
  overflow: hidden;
  width: 100%;
  padding: 14px 24px;
  font-size: 15px;
  font-weight: 700;
  border-radius: 14px;
  border: none;
  background: linear-gradient(45deg, #f44336, #d32f2f);
  color: #fff;
  cursor: pointer;
  letter-spacing: 0.2px;
  box-shadow: 0 10px 22px rgba(211,47,47,0.28);
  transition: transform 0.18s ease, box-shadow 0.25s ease, opacity 0.2s ease;
  margin-top: 0;
}
.aq-feed-btn:disabled { opacity: 0.55; box-shadow: none; }
)rawliteral"
#include "shared/common_bottom_nav_styles.inc"
R"rawliteral(
body { padding-bottom: calc(75px + env(safe-area-inset-bottom, 0px)); }
</style>
</head>
<body>
<div id="toast" class="toast">
  <span class="toast-icon" aria-hidden="true">i</span>
  <span id="toastText" class="toast-text">Годування розпочато</span>
</div>
<div class="hero-header">
)rawliteral"
#include "shared/hero_illustration.inc"
R"rawliteral(
  <div class="app-heading">
    <div class="app-title">AquaFeed</div>
    <div class="app-subtitle" id="homeSubtitle">Автоматична годівниця</div>
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
      <div class="section-title" id="langSectionTitle">Мова інтерфейсу</div>
      <div class="section-subtitle" id="langSectionSubtitle">Оберіть мову веб-інтерфейсу</div>
    </div>
  </div>
  <div class="lang-section-row">
    <button type="button" id="homeLangUkBtn" class="lang-btn">UK</button>
    <button type="button" id="homeLangEnBtn" class="lang-btn">EN</button>
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
      <div class="section-title" id="statusSectionTitle">Статус підключення</div>
      <div class="section-subtitle" id="statusSectionSubtitle">Поточний режим роботи WiFi</div>
    </div>
  </div>
  <div class="status-pill" id="statusPill">
    <span id="statusText">завантаження...</span>
  </div>
</div>

<div class="card">
  <div class="aq-gauges-row" id="gaugesRow"></div>
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
      <div class="section-title" id="scheduleSectionTitle">Розклад годування</div>
      <div class="section-subtitle" id="scheduleSectionSubtitle">Час, повтори та день тижня</div>
    </div>
  </div>
  <div class="aq-schedule-list" id="scheduleList"></div>
  <button type="button" class="aq-add-btn" id="btnAddSlot" onclick="addScheduleSlot()">+ Додати</button>
  <button type="button" class="aq-save-btn" id="btnSaveSchedule" onclick="saveSchedule()" style="display:none;">Зберегти розклад</button>
</div>

<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M13 2L6 13h5l-1 9 8-12h-5l0-8z"></path>
      </svg>
    </div>
    <div>
      <div class="section-title" id="servoSectionTitle">Сервопривід</div>
      <div class="section-subtitle" id="servoSectionSubtitle">Швидкість, кут, кількість повторів</div>
    </div>
  </div>
  <div class="slider-row">
    <label><span id="lblSpeed">Швидкість</span><span id="valSpeed">--</span></label>
    <input type="range" id="speedSlider" min="1" max="20" step="0.5" oninput="onSpeedInput()" onchange="saveSpeed()">
  </div>
  <div class="slider-row" style="margin-top: 14px;">
    <label><span id="lblAngle">Кут</span><span id="valAngle">--</span></label>
    <input type="range" id="angleSlider" min="0" max="180" step="1" oninput="onAngleInput()" onchange="saveAngle()">
  </div>
  <div class="aq-feed-field" style="margin-top: 14px;">
    <span id="lblRepeats">Повторів за замовчуванням</span>
    <div class="aq-stepper">
      <button type="button" class="aq-stepper-btn" onclick="stepRepeats(-1)">−</button>
      <span class="aq-stepper-val" id="repeatsVal">1</span>
      <button type="button" class="aq-stepper-btn" onclick="stepRepeats(1)">+</button>
    </div>
  </div>
</div>

<div class="card">
  <button type="button" class="aq-feed-btn" id="btnFeedNow" onclick="feedNow()">Годувати зараз</button>
  <div class="note-text" id="feedCooldownNote"></div>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
let homeLang = getStoredUiLang();
function homeIsEn() { return homeLang === 'en'; }

function applyHomeLanguage() {
  const subtitle = document.getElementById('homeSubtitle');
  if (subtitle) subtitle.textContent = homeIsEn() ? 'Automatic feeder' : 'Автоматична годівниця';
  const setText = (id, en, uk) => {
    const el = document.getElementById(id);
    if (el) el.textContent = homeIsEn() ? en : uk;
  };
  setText('langSectionTitle', 'Interface language', 'Мова інтерфейсу');
  setText('langSectionSubtitle', 'Choose the web interface language', 'Оберіть мову веб-інтерфейсу');
  setText('statusSectionTitle', 'Connection status', 'Статус підключення');
  setText('statusSectionSubtitle', 'Current WiFi mode', 'Поточний режим роботи WiFi');
  setText('scheduleSectionTitle', 'Feeding schedule', 'Розклад годування');
  setText('scheduleSectionSubtitle', 'Time, repeats, and day of week', 'Час, повтори та день тижня');
  setText('btnAddSlot', '+ Add', '+ Додати');
  setText('btnSaveSchedule', 'Save schedule', 'Зберегти розклад');
  setText('servoSectionTitle', 'Servo', 'Сервопривід');
  setText('servoSectionSubtitle', 'Speed, angle, default repeats', 'Швидкість, кут, кількість повторів');
  setText('lblSpeed', 'Speed', 'Швидкість');
  setText('lblAngle', 'Angle', 'Кут');
  setText('lblRepeats', 'Default repeats', 'Повторів за замовчуванням');
  renderSchedule();
  renderGauges(lastStatus);
  const feedBtn = document.getElementById('btnFeedNow');
  if (feedBtn && !feedBtn.disabled) feedBtn.textContent = homeIsEn() ? 'Feed now' : 'Годувати зараз';
  const tabs = document.querySelectorAll('.bottom-tab span');
  if (tabs[0]) tabs[0].textContent = homeIsEn() ? 'Home' : 'Головна';
  if (tabs[1]) tabs[1].textContent = homeIsEn() ? 'Info' : 'Інформація';
  if (tabs[2]) tabs[2].textContent = homeIsEn() ? 'Settings' : 'Налаштування';
  const ukBtn = document.getElementById('homeLangUkBtn');
  const enBtn = document.getElementById('homeLangEnBtn');
  if (ukBtn) ukBtn.classList.toggle('active', !homeIsEn());
  if (enBtn) enBtn.classList.toggle('active', homeIsEn());
  const heroFish = document.querySelector('.hero-svg-fish');
  if (heroFish) heroFish.setAttribute('aria-label', homeIsEn() ? 'Stylized fish' : 'Стилізована рибка');
}

function setHomeLanguage(lang) {
  homeLang = lang;
  try { localStorage.setItem('aqua_lang', lang); } catch (_) {}
  applyHomeLanguage();
}

// ── Gauges (same geometry as frontend/src/pages/DeviceDashboardPage.tsx) ──
const GAUGE_R = 110, GAUGE_CX = 130, GAUGE_CY = 140, GAUGE_ARC = 180;
const CIRCUMFERENCE = 2 * Math.PI * GAUGE_R;
const TRACK_LEN = (GAUGE_ARC / 360) * CIRCUMFERENCE;
const TRACK_PATH = 'M 20 140 A 110 110 0 0 1 240 140';
let lastStatus = null;

function gaugeSvg(fraction, color, label, sub, animateCharging) {
  const offset = TRACK_LEN * (1 - Math.max(0, Math.min(1, fraction)));
  const arcClass = animateCharging ? 'aq-gauge-fill-arc aq-gauge-charging-arc' : 'aq-gauge-fill-arc';
  const style = animateCharging ? ' style="--charge-full:' + TRACK_LEN + ';--charge-target:' + offset + ';"' : '';
  return '<div class="aq-gauge-wrap">' +
    '<svg class="aq-gauge-svg" viewBox="0 0 260 160">' +
      '<path d="' + TRACK_PATH + '" fill="none" stroke="#E6E9EF" stroke-width="14" stroke-linecap="round"></path>' +
      '<path d="' + TRACK_PATH + '" fill="none" stroke="' + color + '" stroke-width="14" stroke-linecap="round" ' +
        'stroke-dasharray="' + TRACK_LEN + '" stroke-dashoffset="' + offset + '" class="' + arcClass + '"' + style + '></path>' +
      '<text x="130" y="130" text-anchor="middle" dominant-baseline="middle" font-size="28" font-weight="700" fill="' + color + '">' + label + '</text>' +
    '</svg>' +
    '<div class="aq-gauge-title">' + sub + '</div>' +
  '</div>';
}

function renderGauges(j) {
  const row = document.getElementById('gaugesRow');
  if (!row) return;
  if (!j) { row.innerHTML = ''; return; }
  const pct = typeof j.batteryPercent === 'number' ? Math.max(0, Math.min(100, Math.round(j.batteryPercent))) : 0;
  const battColor = pct >= 50 ? '#4CAF50' : pct >= 20 ? '#FF9800' : '#f44336';
  const battHtml = gaugeSvg(pct / 100, battColor, pct + '%', homeIsEn() ? 'Battery' : 'Батарея', !!j.isCharging);

  const minutes = typeof j.nextFeedMinutes === 'number' ? j.nextFeedMinutes : null;
  const nfFraction = minutes != null ? Math.max(0, Math.min(1, 1 - minutes / (24 * 60))) : 0;
  const nfLabel = minutes != null
    ? (Math.floor(minutes / 60) + (homeIsEn() ? 'h' : 'г') + ' ' + (minutes % 60) + (homeIsEn() ? 'm' : 'хв'))
    : '--';
  const nfHtml = gaugeSvg(nfFraction, '#1976D2', nfLabel, homeIsEn() ? 'Until next feed' : 'До годування', false);

  row.innerHTML = battHtml + nfHtml;
}

let schedule = [];
let scheduleDirty = false;
let speedDirty = false;
let angleDirty = false;

// UI day index: 0 = every day, 1..6 = Mon..Sat, 7 = Sun (matches the SPA's
// LocalFeedTime.day convention). Firmware day: -1 = every day, 0 = Sun, 1..6 = Mon..Sat.
const DAY_LABELS_UK = ['Щодня', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Нд'];
const DAY_LABELS_EN = ['Every day', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa', 'Su'];

function dayValueToIndex(d) {
  if (d === -1 || d == null) return 0;
  if (d === 0) return 7;
  return d;
}
function dayIndexToValue(i) {
  if (i === 0) return -1;
  if (i === 7) return 0;
  return i;
}

function renderSchedule() {
  const list = document.getElementById('scheduleList');
  const saveBtn = document.getElementById('btnSaveSchedule');
  if (!list) return;
  const labels = homeIsEn() ? DAY_LABELS_EN : DAY_LABELS_UK;
  const hourOptions = Array.from({ length: 24 }, (_, i) => String(i).padStart(2, '0'));
  const minuteOptions = Array.from({ length: 60 }, (_, i) => String(i).padStart(2, '0'));
  list.innerHTML = schedule.map((slot, idx) => {
    const hourOpts = hourOptions.map((h, i) => '<option value="' + i + '"' + (i === slot.h ? ' selected' : '') + '>' + h + '</option>').join('');
    const minuteOpts = minuteOptions.map((m, i) => '<option value="' + i + '"' + (i === slot.m ? ' selected' : '') + '>' + m + '</option>').join('');
    const dayOpts = labels.map((label, i) => '<option value="' + i + '"' + (dayValueToIndex(slot.d) === i ? ' selected' : '') + '>' + label + '</option>').join('');
    return '<div class="aq-feed-block">' +
      '<div class="aq-feed-time-col"><div class="aq-time-custom">' +
        '<select class="aq-time-part" onchange="updateSlot(' + idx + ',\'h\',parseInt(this.value,10))">' + hourOpts + '</select>' +
        '<span class="aq-time-sep">:</span>' +
        '<select class="aq-time-part" onchange="updateSlot(' + idx + ',\'m\',parseInt(this.value,10))">' + minuteOpts + '</select>' +
      '</div></div>' +
      '<div class="aq-feed-center">' +
        '<div class="aq-feed-field"><span>' + (homeIsEn() ? 'Repeats' : 'Повтори') + '</span>' +
          '<div class="aq-stepper aq-stepper-sm">' +
            '<button type="button" class="aq-stepper-btn aq-stepper-btn-sm" onclick="stepSlotRepeats(' + idx + ',-1)"' + (slot.r <= 1 ? ' disabled' : '') + '>−</button>' +
            '<span class="aq-stepper-val">' + slot.r + '</span>' +
            '<button type="button" class="aq-stepper-btn aq-stepper-btn-sm" onclick="stepSlotRepeats(' + idx + ',1)"' + (slot.r >= 10 ? ' disabled' : '') + '>+</button>' +
          '</div>' +
        '</div>' +
        '<div class="aq-feed-field"><span>' + (homeIsEn() ? 'Day' : 'День') + '</span>' +
          '<select class="aq-day-select" onchange="updateSlot(' + idx + ',\'d\',dayIndexToValue(parseInt(this.value,10)))">' + dayOpts + '</select>' +
        '</div>' +
      '</div>' +
      '<button type="button" class="aq-remove-btn" onclick="removeScheduleSlot(' + idx + ')">×</button>' +
    '</div>';
  }).join('');
  if (saveBtn) saveBtn.style.display = scheduleDirty ? 'flex' : 'none';
}

function updateSlot(idx, field, value) {
  scheduleDirty = true;
  schedule[idx][field] = value;
  renderSchedule();
}

function stepSlotRepeats(idx, delta) {
  const next = Math.max(1, Math.min(10, schedule[idx].r + delta));
  updateSlot(idx, 'r', next);
}

function addScheduleSlot() {
  scheduleDirty = true;
  schedule.push({ h: 10, m: 0, r: 1, d: -1 });
  renderSchedule();
}

function removeScheduleSlot(idx) {
  scheduleDirty = true;
  schedule.splice(idx, 1);
  renderSchedule();
}

function saveSchedule() {
  const payload = schedule.map(s => ({ h: s.h, m: s.m, r: s.r, d: s.d }));
  postForm('/api/setFeedTimes', { data: JSON.stringify(payload) })
    .then(expectOk)
    .then(() => {
      scheduleDirty = false;
      showToastMessage(homeIsEn() ? 'Schedule saved' : 'Розклад збережено');
      renderSchedule();
      updateStatus();
    })
    .catch(error => showToastMessage((homeIsEn() ? 'Save error: ' : 'Помилка збереження: ') + error.message));
}

function onSpeedInput() {
  speedDirty = true;
  const el = document.getElementById('speedSlider');
  const val = document.getElementById('valSpeed');
  if (el && val) val.textContent = parseFloat(el.value).toFixed(1);
}

function saveSpeed() {
  const el = document.getElementById('speedSlider');
  if (!el) return;
  postForm('/api/setSpeed', { speed: el.value })
    .then(expectOk)
    .then(() => { speedDirty = false; showToastMessage(homeIsEn() ? 'Speed saved' : 'Швидкість збережено'); })
    .catch(error => showToastMessage((homeIsEn() ? 'Save error: ' : 'Помилка збереження: ') + error.message));
}

function onAngleInput() {
  angleDirty = true;
  const el = document.getElementById('angleSlider');
  const val = document.getElementById('valAngle');
  if (el && val) val.textContent = el.value + '°';
}

function saveAngle() {
  const el = document.getElementById('angleSlider');
  if (!el) return;
  postForm('/api/setAngle', { angle: el.value })
    .then(expectOk)
    .then(() => { angleDirty = false; showToastMessage(homeIsEn() ? 'Angle saved' : 'Кут збережено'); })
    .catch(error => showToastMessage((homeIsEn() ? 'Save error: ' : 'Помилка збереження: ') + error.message));
}

function stepRepeats(delta) {
  const el = document.getElementById('repeatsVal');
  if (!el) return;
  const next = Math.max(1, Math.min(20, parseInt(el.textContent, 10) + delta));
  el.textContent = next;
  postForm('/api/setRepeats', { repeats: next })
    .then(expectOk)
    .catch(error => showToastMessage((homeIsEn() ? 'Save error: ' : 'Помилка збереження: ') + error.message));
}

function feedNow() {
  const btn = document.getElementById('btnFeedNow');
  if (btn) { btn.disabled = true; }
  postForm('/api/feedNow', {})
    .then(expectOk)
    .then(() => {
      showToastMessage(homeIsEn() ? 'Feeding started' : 'Годування розпочато');
      setTimeout(updateStatus, 1500);
    })
    .catch(error => showToastMessage((homeIsEn() ? 'Feed error: ' : 'Помилка годування: ') + error.message))
    .finally(() => { if (btn) btn.disabled = false; });
}

function updateStatus() {
  fetch('/api/status').then(expectOk).then(r => r.json()).then(j => {
    lastStatus = j;
    const statusText = document.getElementById('statusText');
    const statusPill = document.getElementById('statusPill');
    if (statusPill) statusPill.classList.remove('success', 'warning', 'error');
    if (j.isAPMode) {
      statusText.innerText = (homeIsEn() ? 'Access Point mode (AP)' : 'Режим точки доступу (AP)');
      if (statusPill) statusPill.classList.add('warning');
    } else if (j.wifiIP) {
      statusText.innerText = (homeIsEn() ? 'Connected: ' : 'Підключено: ') + (j.wifiSSID || '') + ' (' + j.wifiIP + ')';
      if (statusPill) statusPill.classList.add('success');
    } else {
      statusText.innerText = homeIsEn() ? 'Not connected' : 'Не підключено';
      if (statusPill) statusPill.classList.add('error');
    }

    renderGauges(j);

    const btn = document.getElementById('btnFeedNow');
    const cooldownNote = document.getElementById('feedCooldownNote');
    const cooldown = j.manualFeedCooldownSeconds || 0;
    if (btn) btn.disabled = cooldown > 0;
    if (cooldownNote) {
      cooldownNote.textContent = cooldown > 0
        ? (homeIsEn() ? ('Wait ' + cooldown + 's before feeding again') : ('Зачекайте ' + cooldown + ' с до наступного годування'))
        : '';
    }

    if (!scheduleDirty && Array.isArray(j.feedTimes)) {
      schedule = j.feedTimes.map(ft => ({ h: ft.h, m: ft.m, r: ft.r, d: typeof ft.d === 'number' ? ft.d : -1 }));
      renderSchedule();
    }
    if (!speedDirty && typeof j.speed === 'number') {
      const speedEl = document.getElementById('speedSlider');
      const speedVal = document.getElementById('valSpeed');
      if (speedEl) speedEl.value = j.speed;
      if (speedVal) speedVal.textContent = j.speed.toFixed(1);
    }
    if (!angleDirty && typeof j.currentAngle === 'number') {
      const angleEl = document.getElementById('angleSlider');
      const angleVal = document.getElementById('valAngle');
      if (angleEl) angleEl.value = j.currentAngle;
      if (angleVal) angleVal.textContent = j.currentAngle + '°';
    }
    const repeatsVal = document.getElementById('repeatsVal');
    if (repeatsVal && typeof j.feedRepeats === 'number') repeatsVal.textContent = j.feedRepeats;
  }).catch(() => {
    const statusText = document.getElementById('statusText');
    if (statusText) statusText.innerText = homeIsEn() ? 'Status load error' : 'Помилка завантаження статусу';
  });
}

window.onload = function() {
  applyHomeLanguage();
  updateStatus();
  const ukBtn = document.getElementById('homeLangUkBtn');
  const enBtn = document.getElementById('homeLangEnBtn');
  if (ukBtn) ukBtn.addEventListener('click', () => setHomeLanguage('uk'));
  if (enBtn) enBtn.addEventListener('click', () => setHomeLanguage('en'));
  setInterval(updateStatus, 5000);
};
document.addEventListener('DOMContentLoaded', function() {
  setActiveBottomTab({ matchRootToHome: true });
});
</script>

)rawliteral"
#include "shared/bottom_tabs_home.inc"
R"rawliteral(</body>
</html>
)rawliteral";
