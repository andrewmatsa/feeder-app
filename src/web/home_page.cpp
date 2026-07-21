// Home page ("/") — always served by the ESP32 itself, independent of the
// backend. Mirrors the React SPA dashboard's Home tab exactly (order, card
// content, styling) — see frontend/src/pages/DeviceDashboardPage.tsx
// renderHome() and frontend/src/App.css .aq-* classes.
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
button:disabled { opacity: 0.5; cursor: not-allowed; }
)rawliteral"
#include "shared/common_card_styles.inc"
R"rawliteral(
.toast {
  position: fixed;
  top: 0; left: 0; right: 0;
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
  width: 18px; height: 18px; border-radius: 50%;
  display: inline-flex; align-items: center; justify-content: center;
  background: rgba(255,255,255,0.2); border: 1px solid rgba(255,255,255,0.35);
  font-size: 12px; font-weight: 700; line-height: 1; flex: 0 0 auto;
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
/* ── Gauges ── */
.aq-battery-card { padding: 20px 16px 14px; }
.aq-gauges-row { display: flex; justify-content: center; gap: 8px; align-items: flex-start; }
.aq-gauge-wrap {
  flex: 1 1 0; min-width: 0; max-width: 160px;
  display: flex; flex-direction: column; align-items: center; gap: 4px;
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
.aq-light-status-on { color: #D97706; }

/* ── Food supply ── */
.aq-food-section { margin-top: 14px; padding-top: 14px; border-top: 1px solid #eef0f3; }
.aq-food-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 10px; }
.aq-food-title { font-size: 13px; font-weight: 600; color: #333; }
.aq-food-bar-track { height: 8px; background: #E6E9EF; border-radius: 999px; overflow: hidden; margin-bottom: 6px; }
.aq-food-bar-fill { height: 100%; border-radius: 999px; transition: width 0.6s ease, background 0.4s; }
.aq-food-meta { display: flex; justify-content: space-between; font-size: 12px; color: #555; font-weight: 500; }
.aq-food-meta-right { color: #999; }
.aq-food-not-set { font-size: 13px; color: #aaa; margin: 6px 0 0; }
.aq-food-form { margin-top: 10px; }
.aq-food-form-row { display: flex; align-items: center; justify-content: space-between; gap: 8px; margin-top: 6px; }
.aq-food-form-label { font-size: 12px; color: #6b7280; }
.aq-food-form-input {
  width: 90px; padding: 6px 8px; border-radius: 8px; border: 1px solid #d5d9e0;
  background: #f9fafc; font-size: 13px; text-align: right; font-family: inherit;
}
.aq-food-save-btn {
  display: inline-flex; align-items: center; justify-content: center;
  width: 100%; margin-top: 8px; padding: 8px 16px; border: none; border-radius: 999px;
  background: #1976D2; color: #fff; font-size: 12px; font-weight: 700; cursor: pointer; font-family: inherit;
}

/* ── Alerts ── */
.aq-alert { display: flex; align-items: center; gap: 8px; padding: 8px 12px; border-radius: 10px; font-size: 13px; font-weight: 600; margin-top: 12px; }
.aq-alert-icon { width: 18px; height: 18px; border-radius: 50%; display: inline-flex; align-items: center; justify-content: center; font-size: 11px; font-weight: 800; flex-shrink: 0; }
.aq-alert-charging { background: rgba(76,175,80,0.14); border: 1px solid rgba(46,125,50,0.35); color: #1b5e20; }
.aq-alert-charging .aq-alert-icon { background: #2e7d32; color: #fff; }
.aq-alert-low { background: rgba(211,47,47,0.12); border: 1px solid rgba(198,40,40,0.35); color: #b71c1c; }
.aq-alert-low .aq-alert-icon { background: #d32f2f; color: #fff; }

/* ── Repeats stepper / Feed button ── */
.aq-repeats-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 14px; }
.aq-stepper { display: inline-flex; align-items: center; gap: 10px; }
.aq-stepper-btn {
  width: 32px; height: 32px; border-radius: 50%; border: 1px solid #d1d5db;
  background: #fff; color: #111827; font-size: 16px; font-weight: 700; line-height: 1;
  cursor: pointer; display: inline-flex; align-items: center; justify-content: center;
  padding: 0; margin-top: 0; transition: background 0.15s;
}
.aq-stepper-btn:active { transform: scale(0.94); }
.aq-stepper-btn:disabled { opacity: 0.4; cursor: not-allowed; }
.aq-stepper-val { min-width: 24px; text-align: center; font-size: 14px; font-weight: 700; color: #111827; }
.aq-stepper-sm { gap: 8px; }
.aq-stepper-btn-sm { width: 28px; height: 28px; font-size: 14px; }
.aq-feed-btn {
  position: relative; overflow: hidden; width: 100%; padding: 14px 24px;
  font-size: 15px; font-weight: 700; border-radius: 14px; border: none;
  background: linear-gradient(45deg, #f44336, #d32f2f); color: #fff; cursor: pointer;
  letter-spacing: 0.2px; box-shadow: 0 10px 22px rgba(211,47,47,0.28);
  transition: transform 0.18s ease, box-shadow 0.25s ease, opacity 0.2s ease; margin-top: 0;
}
.aq-feed-btn:disabled { opacity: 0.7; cursor: not-allowed; }
.aq-feed-btn.is-cooldown { background: #374151; box-shadow: none; flex-direction: column; gap: 2px; }
.aq-feed-btn-cooldown-time { font-size: 22px; font-weight: 700; letter-spacing: 2px; line-height: 1; font-variant-numeric: tabular-nums; }
.aq-feed-btn-cooldown-label { font-size: 11px; font-weight: 500; opacity: 0.75; letter-spacing: 0.3px; }
@keyframes aq-feed-pulse {
  0%   { box-shadow: 0 0 0 0 rgba(244,67,54,0.35); }
  70%  { box-shadow: 0 0 0 10px rgba(244,67,54,0); }
  100% { box-shadow: 0 0 0 0 rgba(244,67,54,0); }
}
.aq-feed-btn.is-feeding { animation: aq-feed-pulse 1.1s ease-in-out infinite; }

/* ── Info banner ── */
.aq-info-banner { margin-bottom: 12px; padding: 10px 14px; background: #f0f7ff; border: 1px solid #c5d9f0; border-radius: 12px; font-size: 13px; color: #374151; line-height: 1.4; }

/* ── Schedule ── */
.aq-schedule-list { display: flex; flex-direction: column; gap: 8px; margin-bottom: 10px; }
.aq-feed-block {
  display: flex; flex-wrap: wrap; gap: 6px 12px; align-items: center;
  padding: 10px 10px; background: rgba(0,0,0,0.03); border-radius: 8px; border-left: 3px solid #8e24aa;
}
.aq-feed-time-col { display: flex; align-items: center; flex-shrink: 0; }
.aq-feed-center { display: flex; flex-direction: column; gap: 4px; align-items: flex-start; flex: 1; min-width: 140px; }
.aq-feed-field { display: inline-flex; align-items: center; gap: 6px; }
.aq-feed-field > span { font-size: 12px; color: #6b7280; }
.aq-time-custom { display: flex; align-items: center; gap: 2px; }
.aq-time-part {
  width: 46px; padding: 4px 2px; border-radius: 6px; text-align: center;
  appearance: none; -webkit-appearance: none; cursor: pointer;
  border: 1px solid #d5d9e0; background: #f9fafc; font-size: 13px; font-weight: 700; color: #1f2937; font-family: inherit;
}
.aq-time-part:focus { outline: none; border-color: #1976D2; box-shadow: 0 0 0 2px rgba(25,118,210,0.12); }
.aq-time-sep { font-size: 15px; font-weight: 700; color: #374151; line-height: 1; }
.aq-day-select {
  width: 88px; padding: 3px 4px; border-radius: 6px; border: 1px solid #d5d9e0;
  background: #f9fafc; font-size: 12px; color: #374151; font-family: inherit; cursor: pointer;
}
.aq-day-select:focus { outline: none; border-color: #1976D2; }
.aq-remove-btn {
  width: 24px; height: 24px; border-radius: 50%; border: none; background: #f44336; color: #fff;
  font-size: 16px; font-weight: 700; line-height: 1; cursor: pointer;
  display: inline-flex; align-items: center; justify-content: center; padding: 0;
  margin-top: 0; margin-left: auto; transition: background 0.15s; flex-shrink: 0;
}
.aq-remove-btn:hover { background: #d32f2f; }
.aq-add-btn {
  display: inline-flex; align-items: center; justify-content: center; width: auto;
  padding: 10px 20px; margin: 4px 0; border: 1px solid rgba(15,23,42,0.12); border-radius: 999px;
  background: #fff; color: #333; font-size: 13px; font-weight: 600; cursor: pointer; font-family: inherit; transition: background 0.15s;
}
.aq-add-btn:hover { background: rgba(15,23,42,0.05); }
.aq-save-btn {
  display: flex; align-items: center; justify-content: center; width: 100%; margin-top: 8px;
  padding: 12px 24px; border: none; border-radius: 999px; background: #111; color: #fff; font-size: 14px; font-weight: 700; cursor: pointer;
}

/* ── Servo sliders ── */
.aq-settings-field { margin-bottom: 4px; }
.aq-settings-label { display: block; font-size: 13px; font-weight: 600; color: #2c3e50; }
.aq-servo-slider { width: 100%; margin-top: 8px; accent-color: #667eea; cursor: pointer; }
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
    <div class="app-subtitle" id="homeSubtitle">Керування годівницею</div>
  </div>
</div>

<!-- 1. Battery card: gauges + food + alerts -->
<div class="card aq-battery-card">
  <div class="aq-gauges-row" id="gaugesRow"></div>

  <div class="aq-food-section" id="foodSection" style="display:none;">
    <div class="aq-food-header"><span class="aq-food-title" id="foodTitle">Запас корму</span></div>
    <div class="aq-food-bar-track"><div class="aq-food-bar-fill" id="foodBarFill" style="width:0%; background:#4CAF50;"></div></div>
    <div class="aq-food-meta">
      <span id="foodRemainingLabel">0 г залишилось</span>
      <span class="aq-food-meta-right" id="foodDurationLabel"></span>
    </div>
    <div class="aq-food-form">
      <div class="aq-food-form-row">
        <span class="aq-food-form-label" id="lblFoodTotal">Завантажено (г)</span>
        <input type="number" min="1" class="aq-food-form-input" id="foodInputTotal" placeholder="г">
      </div>
      <div class="aq-food-form-row">
        <span class="aq-food-form-label" id="lblFoodGpf">Грамів за 1 годівлю</span>
        <input type="number" min="0.1" step="0.1" class="aq-food-form-input" id="foodInputGpf" placeholder="г">
      </div>
      <button type="button" class="aq-food-save-btn" id="btnFoodSave" onclick="saveFoodForm()">Зберегти</button>
    </div>
  </div>

  <div class="aq-alert aq-alert-charging" id="alertCharging" style="display:none;"><span class="aq-alert-icon">⚡</span><span id="alertChargingText">Заряджається</span></div>
  <div class="aq-alert aq-alert-low" id="alertLow" style="display:none;"><span class="aq-alert-icon">!</span><span id="alertLowText">Низький заряд батареї</span></div>
</div>

<!-- 2. Manual feed card -->
<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24"><path d="M12 2a10 10 0 1 0 0 20A10 10 0 0 0 12 2z"></path><path d="M12 8v4l3 3"></path></svg>
    </div>
    <div>
      <div class="section-title" id="manualFeedTitle">Ручне годування</div>
      <div class="section-subtitle" id="manualFeedSub">Швидкий запуск циклу годування</div>
    </div>
  </div>
  <div class="aq-repeats-row">
    <span id="repeatCountLabel">Кількість повторів</span>
    <div class="aq-stepper">
      <button type="button" class="aq-stepper-btn" onclick="stepRepeats(-1)">−</button>
      <span class="aq-stepper-val" id="repeatsVal">1</span>
      <button type="button" class="aq-stepper-btn" onclick="stepRepeats(1)">+</button>
    </div>
  </div>
  <button type="button" class="aq-feed-btn" id="btnFeedNow" onclick="feedNow()">Годувати зараз</button>
</div>

<!-- 3. Schedule card -->
<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"></rect><line x1="16" y1="2" x2="16" y2="6"></line><line x1="8" y1="2" x2="8" y2="6"></line><line x1="3" y1="10" x2="21" y2="10"></line></svg>
    </div>
    <div>
      <div class="section-title" id="scheduleSectionTitle">Автоматичне годування</div>
      <div class="section-subtitle" id="scheduleSectionSubtitle">Налаштуйте розклад годувань</div>
    </div>
  </div>
  <div class="aq-info-banner" id="nextFeedBanner" style="display:none;"></div>
  <div class="aq-schedule-list" id="scheduleList"></div>
  <button type="button" class="aq-add-btn" id="btnAddSlot" onclick="addScheduleSlot()">+ Додати годування</button>
  <button type="button" class="aq-save-btn" id="btnSaveSchedule" onclick="saveSchedule()" style="display:none;">Зберегти всі часи</button>
</div>

<!-- 4. Servo card -->
<div class="card">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24"><line x1="8" y1="5" x2="8" y2="19"></line><line x1="16" y1="5" x2="16" y2="19"></line><circle cx="8" cy="10" r="2.5"></circle><circle cx="16" cy="14" r="2.5"></circle></svg>
    </div>
    <div>
      <div class="section-title" id="servoSectionTitle">Ручне керування</div>
      <div class="section-subtitle" id="servoSectionSubtitle">Ручне керування сервоприводом</div>
    </div>
  </div>
  <div class="aq-settings-field">
    <label class="aq-settings-label" id="lblAngle">Кут серво: 0°</label>
    <input type="range" class="aq-servo-slider" id="angleSlider" min="0" max="180" step="1" oninput="onAngleInput()">
  </div>
  <div class="aq-settings-field" style="margin-top: 16px;">
    <label class="aq-settings-label" id="lblSpeed">Швидкість серво: --</label>
    <input type="range" class="aq-servo-slider" id="speedSlider" min="10" max="20" step="0.5" oninput="onSpeedInput()">
  </div>
  <button type="button" class="aq-save-btn" id="btnSaveSpeed" onclick="saveSpeed()" style="display:none; margin-top: 14px;">Зберегти швидкість</button>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
let homeLang = getStoredUiLang();
function homeIsEn() { return homeLang === 'en'; }

function applyHomeLanguage() {
  const setText = (id, en, uk) => {
    const el = document.getElementById(id);
    if (el) el.textContent = homeIsEn() ? en : uk;
  };
  setText('homeSubtitle', 'Feeder control', 'Керування годівницею');
  setText('foodTitle', 'Food supply', 'Запас корму');
  setText('lblFoodTotal', 'Loaded (g)', 'Завантажено (г)');
  setText('lblFoodGpf', 'Grams per feeding', 'Грамів за 1 годівлю');
  setText('btnFoodSave', 'Save', 'Зберегти');
  setText('alertChargingText', 'Charging', 'Заряджається');
  setText('alertLowText', 'Low battery', 'Низький заряд батареї');
  setText('manualFeedTitle', 'Manual feeding', 'Ручне годування');
  setText('manualFeedSub', 'Quick-start a feeding cycle', 'Швидкий запуск циклу годування');
  setText('repeatCountLabel', 'Number of repeats', 'Кількість повторів');
  setText('scheduleSectionTitle', 'Automatic feeding', 'Автоматичне годування');
  setText('scheduleSectionSubtitle', 'Configure the feeding schedule', 'Налаштуйте розклад годувань');
  setText('btnAddSlot', '+ Add feeding', '+ Додати годування');
  setText('btnSaveSchedule', 'Save all times', 'Зберегти всі часи');
  setText('servoSectionTitle', 'Manual control', 'Ручне керування');
  setText('servoSectionSubtitle', 'Manual servo control', 'Ручне керування сервоприводом');
  const feedBtn = document.getElementById('btnFeedNow');
  if (feedBtn && !feedBtn.disabled) feedBtn.textContent = homeIsEn() ? 'Feed now' : 'Годувати зараз';
  const tabs = document.querySelectorAll('.bottom-tab span');
  if (tabs[0]) tabs[0].textContent = homeIsEn() ? 'Home' : 'Головна';
  if (tabs[1]) tabs[1].textContent = homeIsEn() ? 'Info' : 'Інформація';
  if (tabs[2]) tabs[2].textContent = homeIsEn() ? 'Settings' : 'Налаштування';
  const heroFish = document.querySelector('.hero-svg-fish');
  if (heroFish) heroFish.setAttribute('aria-label', homeIsEn() ? 'Stylized fish' : 'Стилізована рибка');
  renderSchedule();
  renderGauges(lastStatus);
  updateAngleLabel();
  updateSpeedLabel();
  renderFood();
}

// ── Gauges (same geometry as frontend/src/pages/DeviceDashboardPage.tsx) ──
const GAUGE_R = 110, GAUGE_ARC = 180;
const CIRCUMFERENCE = 2 * Math.PI * GAUGE_R;
const TRACK_LEN = (GAUGE_ARC / 360) * CIRCUMFERENCE;
const TRACK_PATH = 'M 20 140 A 110 110 0 0 1 240 140';
let lastStatus = null;

function gaugeSvg(fraction, color, label, sub, subClass, animateCharging) {
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
    (subClass ? '<div class="' + subClass + '"></div>' : '') +
  '</div>';
}

function renderGauges(j) {
  const row = document.getElementById('gaugesRow');
  if (!row) return;
  if (!j) { row.innerHTML = ''; return; }
  const pct = typeof j.batteryPercent === 'number' ? Math.max(0, Math.min(100, Math.round(j.batteryPercent))) : 0;
  const battColor = pct >= 50 ? '#4CAF50' : pct >= 20 ? '#FF9800' : '#f44336';
  const battHtml = gaugeSvg(pct / 100, battColor, pct + '%', homeIsEn() ? 'Battery status' : 'Стан батареї', '', !!j.isCharging);

  const minutes = typeof j.nextFeedMinutes === 'number' ? j.nextFeedMinutes : null;
  const nfFraction = minutes != null ? Math.max(0, Math.min(1, 1 - minutes / (24 * 60))) : 0;
  const nfLabel = minutes != null
    ? (Math.floor(minutes / 60) + (homeIsEn() ? 'h ' : ' год ') + (minutes % 60) + (homeIsEn() ? 'm' : ' хв'))
    : '--';
  const nfHtml = gaugeSvg(nfFraction, '#1976D2', nfLabel, homeIsEn() ? 'Until next feed' : 'До наступного годування', '', false);

  // Light sensor: hardware LDR is disabled on this board (GPIO5/ADC2 unsupported
  // on ESP32-C3), so lightLux never arrives — always render the "off" state,
  // matching the SPA's LightGauge when isOn is false.
  const lightHtml = gaugeSvg(0, '#9CA3AF', '--', homeIsEn() ? 'Light sensor' : 'Сенсор світла', 'aq-gauge-sub', false)
    .replace('<div class="aq-gauge-sub"></div>', '<div class="aq-gauge-sub">' + (homeIsEn() ? 'Off' : 'Вимкнено') + '</div>');

  row.innerHTML = battHtml + nfHtml + lightHtml;
}

// ── Food supply (client-side only, mirrors SPA's localStorage tracker) ──
function foodKey(k) { return 'aq_food_' + k; }
function loadFood() {
  return {
    total: Number(localStorage.getItem(foodKey('total')) || 0),
    gpf: Number(localStorage.getItem(foodKey('gpf')) || 0),
    ts: Number(localStorage.getItem(foodKey('ts')) || 0),
  };
}
function saveFoodForm() {
  const g = parseFloat(document.getElementById('foodInputTotal').value);
  const gpf = parseFloat(document.getElementById('foodInputGpf').value);
  if (!isNaN(g) && g > 0) {
    localStorage.setItem(foodKey('total'), String(g));
    localStorage.setItem(foodKey('ts'), String(Date.now()));
  }
  if (!isNaN(gpf) && gpf > 0) localStorage.setItem(foodKey('gpf'), String(gpf));
  renderFood();
}
function feedTimesPerDay() {
  const everyDaySlots = schedule.filter(s => s.d === -1).length;
  return everyDaySlots || schedule.length || 1;
}
function renderFood() {
  const section = document.getElementById('foodSection');
  if (!section) return;
  const food = loadFood();
  section.style.display = 'block';
  const perDay = feedTimesPerDay();
  let remaining = food.total;
  if (food.total > 0 && food.ts > 0 && food.gpf > 0 && perDay > 0) {
    const daysElapsed = (Date.now() - food.ts) / 86400000;
    remaining = Math.max(0, food.total - daysElapsed * food.gpf * perDay);
  }
  const percent = food.total > 0 ? Math.min(100, (remaining / food.total) * 100) : 0;
  const color = percent > 50 ? '#4CAF50' : percent > 20 ? '#FF9800' : '#f44336';
  document.getElementById('foodBarFill').style.width = percent + '%';
  document.getElementById('foodBarFill').style.background = color;
  document.getElementById('foodRemainingLabel').textContent = Math.round(remaining) + (homeIsEn() ? ' g remaining' : ' г залишилось');
  if (food.gpf > 0 && perDay > 0 && food.total > 0) {
    const totalDays = remaining / (food.gpf * perDay);
    const months = Math.floor(totalDays / 30);
    const days = Math.round(totalDays % 30);
    document.getElementById('foodDurationLabel').textContent = months > 0
      ? (homeIsEn() ? ('~' + months + 'mo ' + days + 'd') : ('~' + months + ' міс ' + days + ' дн'))
      : (homeIsEn() ? ('~' + days + 'd') : ('~' + days + ' дн'));
  } else {
    document.getElementById('foodDurationLabel').textContent = homeIsEn() ? 'No active schedule' : 'Немає активного розкладу';
  }
  const totalInput = document.getElementById('foodInputTotal');
  const gpfInput = document.getElementById('foodInputGpf');
  if (totalInput && document.activeElement !== totalInput) totalInput.value = food.total || '';
  if (gpfInput && document.activeElement !== gpfInput) gpfInput.value = food.gpf || '';
}

let schedule = [];
let scheduleDirty = false;
let speedDirty = false;

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
        '<div class="aq-feed-field"><span>' + (homeIsEn() ? 'Repeats' : 'Повторів') + '</span>' +
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
  renderFood();
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
  renderFood();
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

let angleDebounceTimer = null;

function updateAngleLabel() {
  const el = document.getElementById('angleSlider');
  const lbl = document.getElementById('lblAngle');
  if (el && lbl) lbl.textContent = (homeIsEn() ? 'Servo angle: ' : 'Кут серво: ') + el.value + '°';
}

function onAngleInput() {
  updateAngleLabel();
  if (angleDebounceTimer) clearTimeout(angleDebounceTimer);
  angleDebounceTimer = setTimeout(() => {
    const el = document.getElementById('angleSlider');
    if (!el) return;
    postForm('/api/setAngle', { angle: el.value }).then(expectOk).catch(() => {});
  }, 50);
}

function updateSpeedLabel() {
  const el = document.getElementById('speedSlider');
  const lbl = document.getElementById('lblSpeed');
  if (el && lbl) lbl.textContent = (homeIsEn() ? 'Servo speed: ' : 'Швидкість серво: ') + parseFloat(el.value).toFixed(1);
}

function onSpeedInput() {
  speedDirty = true;
  updateSpeedLabel();
  const saveBtn = document.getElementById('btnSaveSpeed');
  if (saveBtn) saveBtn.style.display = 'flex';
}

function saveSpeed() {
  const el = document.getElementById('speedSlider');
  if (!el) return;
  postForm('/api/setSpeed', { speed: el.value })
    .then(expectOk)
    .then(() => {
      speedDirty = false;
      const saveBtn = document.getElementById('btnSaveSpeed');
      if (saveBtn) saveBtn.style.display = 'none';
      showToastMessage(homeIsEn() ? 'Speed saved' : 'Швидкість збережено');
    })
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

let cooldownInterval = null;

function setFeedButtonCooldown(seconds) {
  const btn = document.getElementById('btnFeedNow');
  if (!btn) return;
  if (cooldownInterval) { clearInterval(cooldownInterval); cooldownInterval = null; }
  if (seconds > 0) {
    let remaining = seconds;
    btn.disabled = true;
    btn.classList.add('is-cooldown');
    const render = () => {
      const mm = String(Math.floor(remaining / 60)).padStart(2, '0');
      const ss = String(remaining % 60).padStart(2, '0');
      btn.innerHTML = '<span class="aq-feed-btn-cooldown-time">' + mm + ':' + ss + '</span>' +
        '<span class="aq-feed-btn-cooldown-label">' + (homeIsEn() ? 'Next feeding' : 'Наступне годування') + '</span>';
    };
    render();
    cooldownInterval = setInterval(() => {
      remaining--;
      if (remaining <= 0) {
        clearInterval(cooldownInterval);
        cooldownInterval = null;
        btn.disabled = false;
        btn.classList.remove('is-cooldown');
        btn.textContent = homeIsEn() ? 'Feed now' : 'Годувати зараз';
      } else {
        render();
      }
    }, 1000);
  } else {
    btn.disabled = false;
    btn.classList.remove('is-cooldown');
    btn.textContent = homeIsEn() ? 'Feed now' : 'Годувати зараз';
  }
}

function feedNow() {
  const btn = document.getElementById('btnFeedNow');
  if (btn) { btn.disabled = true; btn.classList.add('is-feeding'); btn.textContent = homeIsEn() ? 'Feeding...' : 'Годую...'; }
  postForm('/api/feedNow', {})
    .then(expectOk)
    .then(() => {
      showToastMessage(homeIsEn() ? 'Feeding started' : 'Годування розпочато');
      setTimeout(updateStatus, 1500);
    })
    .catch(error => showToastMessage((homeIsEn() ? 'Feed error: ' : 'Помилка годування: ') + error.message))
    .finally(() => { if (btn) btn.classList.remove('is-feeding'); });
}

function updateStatus() {
  fetch('/api/status').then(expectOk).then(r => r.json()).then(j => {
    lastStatus = j;
    renderGauges(j);

    const chargingAlert = document.getElementById('alertCharging');
    const lowAlert = document.getElementById('alertLow');
    if (chargingAlert) chargingAlert.style.display = j.isCharging ? 'flex' : 'none';
    if (lowAlert) lowAlert.style.display = (!j.isCharging && typeof j.batteryPercent === 'number' && j.batteryPercent < 20) ? 'flex' : 'none';

    const cooldown = j.manualFeedCooldownSeconds || 0;
    setFeedButtonCooldown(cooldown);

    const banner = document.getElementById('nextFeedBanner');
    if (banner) {
      if (typeof j.nextFeedMinutes === 'number') {
        banner.style.display = 'block';
        const h = Math.floor(j.nextFeedMinutes / 60), m = j.nextFeedMinutes % 60;
        banner.textContent = (homeIsEn() ? 'Next feeding in: ' : 'До наступного годування: ') + h + (homeIsEn() ? 'h ' : ' год ') + m + (homeIsEn() ? 'm' : ' хв');
      } else {
        banner.style.display = 'none';
      }
    }

    if (!scheduleDirty && Array.isArray(j.feedTimes)) {
      schedule = j.feedTimes.map(ft => ({ h: ft.h, m: ft.m, r: ft.r, d: typeof ft.d === 'number' ? ft.d : -1 }));
      renderSchedule();
      renderFood();
    }
    if (!speedDirty && typeof j.speed === 'number') {
      const speedEl = document.getElementById('speedSlider');
      if (speedEl) { speedEl.value = j.speed; updateSpeedLabel(); }
    }
    const angleEl = document.getElementById('angleSlider');
    if (angleEl && document.activeElement !== angleEl && typeof j.currentAngle === 'number') {
      angleEl.value = j.currentAngle;
      updateAngleLabel();
    }
    const repeatsVal = document.getElementById('repeatsVal');
    if (repeatsVal && typeof j.feedRepeats === 'number') repeatsVal.textContent = j.feedRepeats;
  }).catch(() => {});
}

window.onload = function() {
  applyHomeLanguage();
  updateStatus();
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
