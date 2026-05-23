#include "../wifi_manager.h"

const char* pageWiFiLocked = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>AquaFeed Login</title>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<style>
body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  margin: 0;
  padding: 16px;
  min-height: 100vh;
  background: #F5F5F5;
  color: #222;
  box-sizing: border-box;
}
.shell {
  max-width: 430px;
  margin: 0 auto;
}
.hero-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin-bottom: 18px;
  padding-top: 10px;
}
.app-illustration svg {
  width: 78px;
  height: auto;
  display: block;
  transform: translateY(-6px);
  filter: drop-shadow(0 16px 28px rgba(17, 24, 39, 0.2));
}
.app-illustration {
  display: inline-flex;
  align-items: center;
  justify-content: center;
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
  animation: hero-fish-swim 4.2s ease-in-out infinite;
  transform-origin: 32px 42px;
  transform-box: fill-box;
  outline: none;
  -webkit-tap-highlight-color: transparent;
  cursor: pointer;
}
.hero-fish-body {
  animation: hero-fish-body-wave 2.2s ease-in-out infinite;
  transform-origin: 29px 40px;
  transform-box: fill-box;
}
.hero-fish-tail {
  animation: hero-fish-tail-flick 1.25s ease-in-out infinite;
  transform-origin: 38px 56px;
  transform-box: fill-box;
}
.hero-fish-eye {
  transform-origin: center;
  transform-box: fill-box;
  animation: hero-fish-eye-blink 6s linear infinite;
}
@keyframes hero-fish-swim {
  0%, 100% { transform: translate3d(0px, 0px, 0) rotate(-0.6deg); }
  25% { transform: translate3d(1.8px, -2.4px, 0) rotate(0.5deg); }
  50% { transform: translate3d(3.2px, -0.8px, 0) rotate(0.9deg); }
  75% { transform: translate3d(1px, -3px, 0) rotate(0.1deg); }
}
@keyframes hero-fish-body-wave {
  0%, 100% { transform: rotate(0deg); }
  50% { transform: rotate(1.6deg); }
}
@keyframes hero-fish-tail-flick {
  0%, 100% { transform: rotate(0deg); }
  35% { transform: rotate(-5deg); }
  65% { transform: rotate(4deg); }
}
@keyframes hero-fish-eye-blink {
  0%, 47%, 49%, 100% { transform: scaleY(1); }
  48% { transform: scaleY(0.15); }
}
.app-illustration.is-escaping {
  animation: hero-fish-escape-left 2.1s cubic-bezier(0.22, 0.7, 0.2, 1) 1;
}
.hero-fish-tail.tail-burst {
  animation: hero-fish-tail-burst 0.16s ease-in-out 6;
}
@keyframes hero-fish-escape-left {
  0% { transform: translate3d(0px, 0px, 0); }
  22% { transform: translate3d(-16px, -1px, 0); }
  48% { transform: translate3d(-42px, -3px, 0); }
  72% { transform: translate3d(-10px, 0px, 0); }
  86% { transform: translate3d(6px, 0px, 0); }
  100% { transform: translate3d(0px, 0px, 0); }
}
@keyframes hero-fish-tail-burst {
  0% { transform: rotate(0deg); }
  50% { transform: rotate(-16deg); }
  100% { transform: rotate(14deg); }
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
.card {
  background: #fff;
  border-radius: 18px;
  box-shadow: 0 12px 28px rgba(0, 0, 0, 0.12);
  padding: 22px 18px 18px;
}
.title-lang-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 10px;
}
.title-lang-row h1 { margin: 0; }
.lang-row {
  display: flex;
  gap: 6px;
  flex-shrink: 0;
}
.lang-btn {
  width: auto;
  flex: 0 0 auto;
  border: 1px solid rgba(0,0,0,0.18);
  border-radius: 999px;
  background: #fff;
  color: #555;
  padding: 4px 12px;
  font-size: 12px;
  font-weight: 700;
  font-family: inherit;
  cursor: pointer;
}
.lang-btn.active {
  background: #111;
  color: #fff;
  border-color: #111;
}
h1 {
  margin: 0 0 6px;
  font-size: 20px;
  font-weight: 700;
  color: #1f2937;
}
p {
  margin: 0 0 16px;
  color: #666;
  line-height: 1.45;
  font-size: 13px;
}
label {
  display: block;
  font-weight: 600;
  font-size: 13px;
  margin-bottom: 6px;
  color: #222;
}
input {
  width: 100%;
  box-sizing: border-box;
  padding: 12px;
  border-radius: 12px;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 14px;
  font-family: inherit;
  color: #222;
  margin-bottom: 14px;
  outline: none;
  transition: border-color 0.15s, box-shadow 0.15s;
}
input:focus {
  border-color: #1976D2;
  box-shadow: 0 0 0 3px rgba(25, 118, 210, 0.12);
}
button {
  width: 100%;
  border: none;
  border-radius: 999px;
  background: #111;
  color: #fff;
  padding: 14px;
  font-size: 15px;
  font-weight: 700;
  font-family: inherit;
  cursor: pointer;
  transition: opacity 0.15s;
}
button:hover { opacity: 0.85; }
.note {
  margin-top: 14px;
  font-size: 13px;
  color: #6b7280;
  line-height: 1.45;
}
#msg {
  min-height: 20px;
  margin-top: 12px;
  font-size: 13px;
  color: #c62828;
}
</style>
</head>
<body>
  <div class="shell">
    <div class="hero-header">
      <div class="app-heading">
        <div class="app-title">AquaFeed Control</div>
        <div class="app-subtitle">Вхід у WiFi налаштування</div>
      </div>
      <div class="app-illustration">
        <svg class="hero-svg-fish" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="Стилізована рибка">
          <path class="hero-fish-body" d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4" fill="#728389"/>
          <g fill="#8d9ba3">
            <path class="hero-fish-tail" d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"/>
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
          <ellipse class="hero-fish-eye" cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c"/>
        </svg>
      </div>
    </div>
    <div class="card">
      <div class="title-lang-row">
        <h1 id="title">Розблокувати налаштування</h1>
        <div class="lang-row">
          <button type="button" id="langUkBtn" class="lang-btn" onclick="setLang('uk')">UK</button>
          <button type="button" id="langEnBtn" class="lang-btn" onclick="setLang('en')">EN</button>
        </div>
      </div>
      <p id="desc">Введіть пароль точки доступу, щоб перейти до налаштування домашнього WiFi.</p>
      <label id="passwordLabel" for="password">Пароль точки доступу</label>
      <input id="password" type="password" placeholder="12345678">
      <button type="button" id="unlockBtn" onclick="login()">Розблокувати</button>
      <div id="noteText" class="note">Після входу ви побачите окремий екран підключення WiFi для введення даних домашньої мережі.</div>
      <div id="msg"></div>
    </div>
  </div>
<script>
let apLang = localStorage.getItem('aqua_lang') === 'en' ? 'en' : 'uk';
const AP_I18N = {
  uk: {
    subtitle: 'Вхід у WiFi налаштування',
    title: 'Розблокувати налаштування',
    desc: 'Введіть пароль точки доступу, щоб перейти до налаштування домашнього WiFi.',
    passwordLabel: 'Пароль точки доступу',
    passwordPlaceholder: '12345678',
    unlockBtn: 'Розблокувати',
    note: 'Після входу ви побачите окремий екран підключення WiFi для введення даних домашньої мережі.',
    wrongPassword: 'Невірний пароль',
    loginFailed: 'Помилка входу'
  },
  en: {
    subtitle: 'WiFi access login',
    title: 'Unlock settings',
    desc: 'Enter the access point password to continue to WiFi connection setup.',
    passwordLabel: 'Access point password',
    passwordPlaceholder: '12345678',
    unlockBtn: 'Unlock settings',
    note: 'After login you will see a separate WiFi connection screen for entering your home network credentials.',
    wrongPassword: 'Wrong password',
    loginFailed: 'Login failed'
  }
};
function apT(key) {
  const dict = AP_I18N[apLang] || AP_I18N.uk;
  return dict[key] || key;
}
function applyApLang() {
  const subtitle = document.querySelector('.app-subtitle');
  if (subtitle) subtitle.textContent = apT('subtitle');
  const title = document.getElementById('title');
  if (title) title.textContent = apT('title');
  const desc = document.getElementById('desc');
  if (desc) desc.textContent = apT('desc');
  const passwordLabel = document.getElementById('passwordLabel');
  if (passwordLabel) passwordLabel.textContent = apT('passwordLabel');
  const passwordInput = document.getElementById('password');
  if (passwordInput) passwordInput.placeholder = apT('passwordPlaceholder');
  const unlockBtn = document.getElementById('unlockBtn');
  if (unlockBtn) unlockBtn.textContent = apT('unlockBtn');
  const noteText = document.getElementById('noteText');
  if (noteText) noteText.textContent = apT('note');
  const ukBtn = document.getElementById('langUkBtn');
  const enBtn = document.getElementById('langEnBtn');
  if (ukBtn) ukBtn.classList.toggle('active', apLang === 'uk');
  if (enBtn) enBtn.classList.toggle('active', apLang === 'en');
}
function setLang(lang) {
  apLang = lang === 'en' ? 'en' : 'uk';
  localStorage.setItem('aqua_lang', apLang);
  applyApLang();
}
function login() {
  const password = document.getElementById('password').value;
  const msg = document.getElementById('msg');
  msg.textContent = '';
  fetch('/api/apLogin', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8',
      'X-AquaFeed-Client': 'webui',
    },
    body: new URLSearchParams({ password }).toString(),
  }).then(async (response) => {
    if (!response.ok) {
      throw new Error(await response.text() || apT('loginFailed'));
    }
    window.location.href = '/wifi';
  }).catch((error) => {
    msg.textContent = error.message === 'invalid password' ? apT('wrongPassword') : error.message;
  });
}
document.getElementById('password').addEventListener('keydown', (event) => {
  if (event.key === 'Enter') login();
});
let _hTbt=null,_hEt=null;
document.addEventListener('DOMContentLoaded',()=>{applyApLang();const f=document.querySelector('.hero-svg-fish'),t=document.querySelector('.hero-fish-tail'),i=f&&f.closest('.app-illustration');if(!f||!t||!i||f.dataset.tbi)return;f.dataset.tbi='1';f.addEventListener('click',()=>{if(i.classList.contains('is-escaping'))return;i.classList.add('is-escaping');t.classList.remove('tail-burst');void t.offsetWidth;t.classList.add('tail-burst');if(_hTbt)clearTimeout(_hTbt);_hTbt=setTimeout(()=>{t.classList.remove('tail-burst');_hTbt=null;},950);if(_hEt)clearTimeout(_hEt);_hEt=setTimeout(()=>{i.classList.remove('is-escaping');_hEt=null;},2150);});});
</script>
</body>
</html>
)rawliteral";

const char* pageWiFiConnect = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>AquaFeed WiFi Setup</title>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<style>
body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  margin: 0;
  padding: 16px;
  min-height: 100vh;
  background: #F5F5F5;
  color: #222;
  box-sizing: border-box;
}
.shell {
  max-width: 430px;
  margin: 0 auto;
}
.hero-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin-bottom: 18px;
  padding-top: 10px;
}
.app-illustration svg {
  width: 78px;
  height: auto;
  display: block;
  transform: translateY(-6px);
  filter: drop-shadow(0 16px 28px rgba(17, 24, 39, 0.2));
}
.app-illustration {
  display: inline-flex;
  align-items: center;
  justify-content: center;
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
  animation: hero-fish-swim 4.2s ease-in-out infinite;
  transform-origin: 32px 42px;
  transform-box: fill-box;
  outline: none;
  -webkit-tap-highlight-color: transparent;
  cursor: pointer;
}
.hero-fish-body {
  animation: hero-fish-body-wave 2.2s ease-in-out infinite;
  transform-origin: 29px 40px;
  transform-box: fill-box;
}
.hero-fish-tail {
  animation: hero-fish-tail-flick 1.25s ease-in-out infinite;
  transform-origin: 38px 56px;
  transform-box: fill-box;
}
.hero-fish-eye {
  transform-origin: center;
  transform-box: fill-box;
  animation: hero-fish-eye-blink 6s linear infinite;
}
@keyframes hero-fish-swim {
  0%, 100% { transform: translate3d(0px, 0px, 0) rotate(-0.6deg); }
  25% { transform: translate3d(1.8px, -2.4px, 0) rotate(0.5deg); }
  50% { transform: translate3d(3.2px, -0.8px, 0) rotate(0.9deg); }
  75% { transform: translate3d(1px, -3px, 0) rotate(0.1deg); }
}
@keyframes hero-fish-body-wave {
  0%, 100% { transform: rotate(0deg); }
  50% { transform: rotate(1.6deg); }
}
@keyframes hero-fish-tail-flick {
  0%, 100% { transform: rotate(0deg); }
  35% { transform: rotate(-5deg); }
  65% { transform: rotate(4deg); }
}
@keyframes hero-fish-eye-blink {
  0%, 47%, 49%, 100% { transform: scaleY(1); }
  48% { transform: scaleY(0.15); }
}
.app-illustration.is-escaping {
  animation: hero-fish-escape-left 2.1s cubic-bezier(0.22, 0.7, 0.2, 1) 1;
}
.hero-fish-tail.tail-burst {
  animation: hero-fish-tail-burst 0.16s ease-in-out 6;
}
@keyframes hero-fish-escape-left {
  0% { transform: translate3d(0px, 0px, 0); }
  22% { transform: translate3d(-16px, -1px, 0); }
  48% { transform: translate3d(-42px, -3px, 0); }
  72% { transform: translate3d(-10px, 0px, 0); }
  86% { transform: translate3d(6px, 0px, 0); }
  100% { transform: translate3d(0px, 0px, 0); }
}
@keyframes hero-fish-tail-burst {
  0% { transform: rotate(0deg); }
  50% { transform: rotate(-16deg); }
  100% { transform: rotate(14deg); }
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
.card {
  background: #fff;
  border-radius: 18px;
  box-shadow: 0 12px 28px rgba(0, 0, 0, 0.12);
  padding: 22px 18px 18px;
}
.title-lang-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 10px;
}
.title-lang-row h1 { margin: 0; }
.lang-row {
  display: flex;
  gap: 6px;
  flex-shrink: 0;
}
.lang-btn {
  width: auto;
  flex: 0 0 auto;
  border: 1px solid rgba(0,0,0,0.18);
  border-radius: 999px;
  background: #fff;
  color: #555;
  padding: 4px 12px;
  font-size: 12px;
  font-weight: 700;
  font-family: inherit;
  cursor: pointer;
}
.lang-btn.active {
  background: #111;
  color: #fff;
  border-color: #111;
}
h1 {
  margin: 0 0 6px;
  font-size: 20px;
  font-weight: 700;
  color: #1f2937;
}
p {
  margin: 0 0 16px;
  color: #666;
  line-height: 1.45;
  font-size: 13px;
}
label {
  display: block;
  font-weight: 600;
  font-size: 13px;
  margin-bottom: 6px;
  color: #222;
}
input {
  width: 100%;
  box-sizing: border-box;
  padding: 12px;
  border-radius: 12px;
  border: 1px solid #d5d9e0;
  background: #f9fafc;
  font-size: 14px;
  font-family: inherit;
  color: #222;
  margin-bottom: 14px;
  outline: none;
  transition: border-color 0.15s, box-shadow 0.15s;
}
input:focus {
  border-color: #1976D2;
  box-shadow: 0 0 0 3px rgba(25, 118, 210, 0.12);
}
button {
  width: 100%;
  border: none;
  border-radius: 999px;
  background: #111;
  color: #fff;
  padding: 14px;
  font-size: 15px;
  font-weight: 700;
  font-family: inherit;
  cursor: pointer;
  transition: opacity 0.15s;
}
button:hover { opacity: 0.85; }
.hint {
  margin-top: 12px;
  font-size: 13px;
  color: #6b7280;
  line-height: 1.45;
}
#msg {
  min-height: 20px;
  margin-top: 12px;
  font-size: 13px;
  color: #c62828;
  white-space: pre-line;
}
.post-connect-actions {
  display: none;
  margin-top: 12px;
  gap: 8px;
}
.post-connect-actions button {
  margin-top: 0;
}
</style>
</head>
<body>
  <div class="shell">
    <div class="hero-header">
      <div class="app-heading">
        <div class="app-title">AquaFeed Control</div>
        <div class="app-subtitle">Підключення WiFi</div>
      </div>
      <div class="app-illustration">
        <svg class="hero-svg-fish" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="Стилізована рибка">
          <path class="hero-fish-body" d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4" fill="#728389"/>
          <g fill="#8d9ba3">
            <path class="hero-fish-tail" d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"/>
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
          <ellipse class="hero-fish-eye" cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c"/>
        </svg>
      </div>
    </div>
    <div class="card">
      <div class="title-lang-row">
        <h1 id="title">Підключіть домашній WiFi</h1>
        <div class="lang-row">
          <button type="button" id="langUkBtn" class="lang-btn" onclick="setConnectLang('uk')">UK</button>
          <button type="button" id="langEnBtn" class="lang-btn" onclick="setConnectLang('en')">EN</button>
        </div>
      </div>
      <p id="desc">Введіть дані домашнього WiFi. Після успішного підключення відкриється повна сторінка налаштувань WiFi.</p>
      <label id="ssidLabel" for="ssid">Назва WiFi мережі</label>
      <input id="ssid" type="text" placeholder="МійДомашнійWiFi">
      <label id="passwordLabel" for="password">Пароль WiFi</label>
      <input id="password" type="password" placeholder="Введіть пароль WiFi">
      <button type="button" id="connectBtn" onclick="connectWifi()">Підключити і розблокувати</button>
      <div id="hintText" class="hint">Після успішного підключення відʼєднайтеся від точки доступу AquaFeed, підʼєднайте телефон до домашнього WiFi і відкрийте <strong>http://fish.local/</strong>.</div>
      <div id="msg"></div>
      <div id="postConnectActions" class="post-connect-actions">
        <button type="button" id="openFishLocalBtn" onclick="openProvisionedWifiPage()">Open fish.local</button>
      </div>
    </div>
  </div>
<script>
let provisionedWifiPageUrl = 'http://fish.local/';
let provisionStatusPoll = null;
let connectLang = localStorage.getItem('aqua_lang') === 'en' ? 'en' : 'uk';
const CONNECT_I18N = {
  uk: {
    subtitle: 'Підключення WiFi',
    title: 'Підключіть домашній WiFi',
    desc: 'Введіть дані домашнього WiFi. Після успішного підключення відкриється повна сторінка налаштувань WiFi.',
    ssidLabel: 'Назва WiFi мережі',
    ssidPlaceholder: 'МійДомашнійWiFi',
    passwordLabel: 'Пароль WiFi',
    passwordPlaceholder: 'Введіть пароль WiFi',
    connectBtn: 'Підключити і розблокувати',
    hint: 'Після успішного підключення відʼєднайтеся від точки доступу AquaFeed, підʼєднайте телефон до домашнього WiFi і відкрийте <strong>http://fish.local/</strong>.',
    openFishLocalBtn: 'Відкрити fish.local',
    enterSsid: 'Введіть назву WiFi мережі',
    connecting: 'Підключення...',
    connectedPrefix: 'Підключено до ',
    connectedFollowUp: 'Відʼєднайтеся від точки доступу AquaFeed, підʼєднайте телефон до домашнього WiFi і відкрийте http://fish.local/',
    connectedIpPrefix: ' або http://',
    connectedIpSuffix: '/',
    failed: 'Не вдалося підключитися до цього WiFi. Перевірте SSID/пароль і спробуйте ще раз.',
    progressPrefix: 'Підключення до ',
    progressSuffix: '...\nЗалиште цю сторінку відкритою, доки пристрій не повідомить про успішне підключення.',
    fetchLost: 'Точка доступу вимкнулась до отримання фінальної відповіді браузером.\nЯкщо пристрій підключився до вашого домашнього WiFi, відʼєднайтеся від точки доступу AquaFeed, підʼєднайте телефон до домашнього WiFi і відкрийте http://fish.local/',
    wifiConnectionFailed: 'Не вдалося підключитися до цього WiFi. Перевірте SSID/пароль і спробуйте ще раз.',
    genericConnectFailed: 'Не вдалося підключитися до WiFi'
  },
  en: {
    subtitle: 'WiFi connection',
    title: 'Connect home WiFi',
    desc: 'Enter your home WiFi credentials. After a successful connection you will get access to the full WiFi settings page.',
    ssidLabel: 'WiFi network name',
    ssidPlaceholder: 'MyHomeWiFi',
    passwordLabel: 'WiFi password',
    passwordPlaceholder: 'Enter WiFi password',
    connectBtn: 'Connect and unlock',
    hint: 'If connection succeeds, disconnect from the AquaFeed setup access point, reconnect your phone to your home WiFi, and then open <strong>http://fish.local/</strong>.',
    openFishLocalBtn: 'Open fish.local',
    enterSsid: 'Enter WiFi network name',
    connecting: 'Connecting...',
    connectedPrefix: 'Connected to ',
    connectedFollowUp: 'Disconnect from the AquaFeed setup access point, reconnect your phone to your home WiFi, and open http://fish.local/',
    connectedIpPrefix: ' or http://',
    connectedIpSuffix: '/',
    failed: 'Could not connect to this WiFi. Check SSID/password and try again.',
    progressPrefix: 'Connecting to ',
    progressSuffix: '...\nKeep this page open until the device reports Connected.',
    fetchLost: 'The setup access point disconnected before the browser received the final reply.\nIf the device joined your home WiFi, disconnect from the AquaFeed setup access point, reconnect your phone to your home WiFi, and open http://fish.local/',
    wifiConnectionFailed: 'Could not connect to this WiFi. Check SSID/password and try again.',
    genericConnectFailed: 'WiFi connection failed'
  }
};
function ct(key) {
  const dict = CONNECT_I18N[connectLang] || CONNECT_I18N.uk;
  return dict[key] || key;
}
function applyConnectLang() {
  const subtitle = document.querySelector('.app-subtitle');
  if (subtitle) subtitle.textContent = ct('subtitle');
  const title = document.getElementById('title');
  if (title) title.textContent = ct('title');
  const desc = document.getElementById('desc');
  if (desc) desc.textContent = ct('desc');
  const ssidLabel = document.getElementById('ssidLabel');
  if (ssidLabel) ssidLabel.textContent = ct('ssidLabel');
  const ssid = document.getElementById('ssid');
  if (ssid) ssid.placeholder = ct('ssidPlaceholder');
  const passwordLabel = document.getElementById('passwordLabel');
  if (passwordLabel) passwordLabel.textContent = ct('passwordLabel');
  const password = document.getElementById('password');
  if (password) password.placeholder = ct('passwordPlaceholder');
  const connectBtn = document.getElementById('connectBtn');
  if (connectBtn) connectBtn.textContent = ct('connectBtn');
  const hint = document.getElementById('hintText');
  if (hint) hint.innerHTML = ct('hint');
  const openBtn = document.getElementById('openFishLocalBtn');
  if (openBtn) openBtn.textContent = ct('openFishLocalBtn');
  const ukBtn = document.getElementById('langUkBtn');
  const enBtn = document.getElementById('langEnBtn');
  if (ukBtn) ukBtn.classList.toggle('active', connectLang === 'uk');
  if (enBtn) enBtn.classList.toggle('active', connectLang === 'en');
}
function setConnectLang(lang) {
  connectLang = lang === 'en' ? 'en' : 'uk';
  localStorage.setItem('aqua_lang', connectLang);
  applyConnectLang();
}

function setProvisionedWifiPageUrl(url) {
  provisionedWifiPageUrl = url || 'http://fish.local/';
}

function togglePostConnectActions(visible) {
  const actions = document.getElementById('postConnectActions');
  if (!actions) return;
  actions.style.display = visible ? 'flex' : 'none';
}

function openProvisionedWifiPage() {
  window.location.href = provisionedWifiPageUrl || 'http://fish.local/';
}

function stopProvisionStatusPolling() {
  if (provisionStatusPoll) {
    clearInterval(provisionStatusPoll);
    provisionStatusPoll = null;
  }
}

function applyProvisionStatus(data) {
  const msg = document.getElementById('msg');
  if (!msg || !data) return;

  if (data.status === 'idle') {
    stopProvisionStatusPolling();
    return;
  }

  if (data.status === 'connected') {
    stopProvisionStatusPolling();
    msg.style.color = '#2e7d32';
    setProvisionedWifiPageUrl('http://fish.local/');
    msg.textContent = ct('connectedPrefix') + (data.ssid || 'WiFi') + '.\n' + ct('connectedFollowUp') + (data.ip ? ct('connectedIpPrefix') + data.ip + ct('connectedIpSuffix') : '');
    togglePostConnectActions(true);
    return;
  }

  if (data.status === 'failed') {
    stopProvisionStatusPolling();
    msg.style.color = '#c62828';
    msg.textContent = ct('failed');
    togglePostConnectActions(false);
    return;
  }

  msg.style.color = '#b26a00';
  msg.textContent = ct('progressPrefix') + (data.ssid || 'WiFi') + ct('progressSuffix');
}

function pollProvisionStatus() {
  fetch('/api/provisionWiFiStatus', {
    headers: {
      'X-AquaFeed-Client': 'webui',
    },
  }).then(r => r.json())
    .then(data => applyProvisionStatus(data))
    .catch(() => {});
}

function startProvisionStatusPolling() {
  stopProvisionStatusPolling();
  pollProvisionStatus();
  provisionStatusPoll = setInterval(pollProvisionStatus, 1000);
}

function connectWifi() {
  const ssid = document.getElementById('ssid').value.trim();
  const password = document.getElementById('password').value;
  const msg = document.getElementById('msg');
  if (!ssid) {
    togglePostConnectActions(false);
    msg.textContent = ct('enterSsid');
    return;
  }
  setProvisionedWifiPageUrl('http://fish.local/');
  togglePostConnectActions(false);
  msg.style.color = '#b26a00';
  msg.textContent = ct('connecting');
  fetch('/api/provisionWiFi', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8',
      'X-AquaFeed-Client': 'webui',
    },
    body: new URLSearchParams({ ssid, password }).toString(),
  }).then(async (response) => {
    const text = await response.text();
    let data = null;
    try {
      data = JSON.parse(text);
    } catch (_) {}
    if (!response.ok || !data || !data.ok) {
      throw new Error((data && data.message) || text || ct('genericConnectFailed'));
    }
    startProvisionStatusPolling();
  }).catch((error) => {
    msg.style.color = '#c62828';
    if (error && error.message === 'Failed to fetch') {
      msg.style.color = '#b26a00';
      setProvisionedWifiPageUrl('http://fish.local/');
      msg.textContent = ct('fetchLost');
      togglePostConnectActions(true);
      return;
    }
    togglePostConnectActions(false);
    msg.textContent = error.message === 'wifi connection failed'
      ? ct('wifiConnectionFailed')
      : error.message;
  });
}
document.getElementById('password').addEventListener('keydown', (event) => {
  if (event.key === 'Enter') connectWifi();
});
applyConnectLang();
pollProvisionStatus();
let _hTbt2=null,_hEt2=null;
document.addEventListener('DOMContentLoaded',()=>{const f=document.querySelector('.hero-svg-fish'),t=document.querySelector('.hero-fish-tail'),i=f&&f.closest('.app-illustration');if(!f||!t||!i||f.dataset.tbi)return;f.dataset.tbi='1';f.addEventListener('click',()=>{if(i.classList.contains('is-escaping'))return;i.classList.add('is-escaping');t.classList.remove('tail-burst');void t.offsetWidth;t.classList.add('tail-burst');if(_hTbt2)clearTimeout(_hTbt2);_hTbt2=setTimeout(()=>{t.classList.remove('tail-burst');_hTbt2=null;},950);if(_hEt2)clearTimeout(_hEt2);_hEt2=setTimeout(()=>{i.classList.remove('is-escaping');_hEt2=null;},2150);});});
</script>
</body>
</html>
)rawliteral";

const char* pageWiFi = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Налаштування WiFi</title>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<style>
)rawliteral"
#include "shared/common_body_base_styles.inc"
R"rawliteral(
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
select {
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
select:focus {
  outline: none;
  border-color: #1976D2;
  box-shadow: 0 0 0 3px rgba(25, 118, 210, 0.15);
}
#deepSleepIdleSec,
#displayOffSec,
#feedIntervalMin {
  width: 100px;
  max-width: 100%;
  margin-top: 6px;
}
#timezoneOffsetMin {
  width: 100%;
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
.toast-text {
  display: inline-block;
}
.toast.show {
  opacity: 1;
  transform: translateY(0) scale(1);
}
)rawliteral"
#include "shared/common_section_styles.inc"
R"rawliteral(
.section-header {
  margin-bottom: 16px;
}
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
.login-note {
  margin-top: 10px;
}
.auth-gated-card {
  transition: opacity 0.2s ease;
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
  justify-content: space-between;
  gap: 12px;
  cursor: pointer;
  user-select: none;
  margin: 12px 0;
  width: 100%;
}
.power-toggle input {
  position: absolute;
  opacity: 0;
  pointer-events: none;
}
.power-toggle-box {
  width: 26px;
  height: 26px;
  order: 2;
  margin-left: auto;
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
  order: 1;
  font-size: 13px;
  color: #333;
  font-weight: 500;
}
.power-toggle-label {
  order: 1;
  display: inline-flex;
  align-items: center;
  gap: 8px;
}
.power-toggle-inline-icon {
  width: 16px;
  height: 16px;
  stroke: currentColor;
  stroke-width: 1.8;
  fill: none;
  stroke-linecap: round;
  stroke-linejoin: round;
  color: #4A5568;
  flex: 0 0 auto;
}
.power-toggle:hover .power-toggle-box {
  border-color: #b8c2d1;
}
)rawliteral"
#include "shared/common_bottom_nav_styles.inc"
R"rawliteral(
.bottom-tab-icon {
  display: inline-block;
}
body {
  padding-bottom: calc(75px + env(safe-area-inset-bottom, 0px));
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
    <div class="app-subtitle">Налаштування WiFi</div>
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
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
      <div class="section-subtitle">Оберіть мову веб-інтерфейсу</div>
    </div>
  </div>
  <div class="lang-section-row">
    <button type="button" id="wifiLangUkBtn" class="lang-btn">UK</button>
    <button type="button" id="wifiLangEnBtn" class="lang-btn">EN</button>
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
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
</div>

<div class="card" id="apLoginCard" style="display:none;">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 17a2 2 0 1 0 0-4"></path>
        <path d="M7 10V8a5 5 0 0 1 10 0v2"></path>
        <rect x="5" y="10" width="14" height="10" rx="2"></rect>
      </svg>
    </div>
    <div>
      <div class="section-title" id="apLoginTitle">Підтвердження доступу</div>
      <div class="section-subtitle" id="apLoginSubtitle">Для зміни налаштувань введіть пароль точки доступу</div>
    </div>
  </div>
  <div class="row">
    <label id="apLoginLabel" for="apLoginPassword">Пароль точки доступу:</label>
    <input type="password" id="apLoginPassword" placeholder="Введіть пароль точки доступу" style="width: 100%;">
  </div>
  <div class="button-row">
    <button type="button" id="btnApLogin" onclick="loginAp()">Увійти</button>
  </div>
  <div class="note-text login-note" id="apLoginNote">
    Використайте пароль точки доступу AquaFeed, щоб відкрити локальні налаштування в режимі AP.
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <path d="M12 17a2 2 0 1 0 0-4"></path>
        <path d="M5 10V8a7 7 0 0 1 14 0v2"></path>
        <rect x="5" y="10" width="14" height="10" rx="2"></rect>
      </svg>
    </div>
    <div>
      <div class="section-title">Підключення до WiFi</div>
      <div class="section-subtitle" id="wifiNetworkSubtitle">Управління збереженою мережею</div>
    </div>
  </div>
  <button type="button" class="secondary" id="btnWifiForget" onclick="forgetWiFi()">Забути мережу</button>
  <div class="note-text" id="wifiNoteNetwork">
    Кнопка «Забути» видаляє збережені дані мережі та повертає пристрій у режим точки доступу за адресами <code>http://192.168.4.1</code> або <code>http://fish.local</code>.
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
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
    <span class="power-toggle-label">
      <svg class="power-toggle-inline-icon" viewBox="0 0 24 24" aria-hidden="true">
        <path d="M13 2L6 13h5l-1 9 8-12h-5l0-8z"></path>
      </svg>
      <span class="power-toggle-text" id="wifiLblPowerEco">Режим економії енергії</span>
    </span>
  </label>
  <div class="note-text" id="wifiNotePower"></div>
  <div class="row" id="deepSleepIdleRow" style="margin-top: 12px;">
    <label for="deepSleepIdleSec" id="labelDeepSleepIdle">Секунд бездіяльності до глибокого сну (10–3600):</label>
    <input type="number" id="deepSleepIdleSec" min="10" max="3600" value="300">
  </div>
  <div class="note-text" id="wifiNotePowerOff" style="margin-top: 12px;"></div>
  <label class="power-toggle" style="margin-top: 16px;">
    <input type="checkbox" id="displayEnabled">
    <span class="power-toggle-box"></span>
    <span class="power-toggle-label">
      <svg class="power-toggle-inline-icon" viewBox="0 0 24 24" aria-hidden="true">
        <rect x="3.5" y="5.5" width="17" height="11" rx="2"></rect>
        <path d="M9 19h6"></path>
        <path d="M12 16.5v2.5"></path>
      </svg>
      <span class="power-toggle-text" id="wifiLblDisplayToggle">Увімкнути OLED дисплей</span>
    </span>
  </label>
  <div class="note-text" id="wifiNoteOled">Вимкніть дисплей для економії енергії. Всі функції працюють через веб-інтерфейс.</div>
  <div class="row" id="displayOffRow" style="margin-top: 12px;">
    <label for="displayOffSec" id="lblDisplayOffSec">Секунд до вимкнення OLED (режим економії, 5–600):</label>
    <input type="number" id="displayOffSec" min="5" max="600" value="20">
  </div>
  <div class="note-text" id="wifiNoteDisplayOff" style="font-size: 12px; color: #6b7280;"></div>
  <div class="button-row" style="margin-top: 12px;">
    <button type="button" id="btnSaveEnergySettings" onclick="saveEnergySettings()">Зберегти налаштування економії енергії</button>
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
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
      <div class="section-title" id="feedIntervalSectionTitle">Інтервал годування</div>
      <div class="section-subtitle" id="feedIntervalSectionSubtitle">Окреме обмеження частоти запуску годувань</div>
    </div>
  </div>
  <div class="row" id="feedIntervalRow">
    <label for="feedIntervalMin" id="labelFeedIntervalMin">Мінімальний інтервал між годуваннями (хв, 1–1440):</label>
    <input type="number" id="feedIntervalMin" min="1" max="1440" value="5">
  </div>
  <div class="note-text" id="wifiNoteFeedInterval" style="font-size: 12px; color: #6b7280;"></div>
  <div class="button-row" style="margin-top: 12px;">
    <button type="button" id="btnSaveFeedInterval" onclick="saveFeedInterval()">Зберегти інтервал годування</button>
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <rect x="3.5" y="5.5" width="17" height="11" rx="2"></rect>
        <path d="M9 19h6"></path>
        <path d="M12 16.5v2.5"></path>
      </svg>
    </div>
    <div>
      <div class="section-title" id="batteryCalSectionTitle">Калібрування батареї</div>
      <div class="section-subtitle" id="batteryCalSectionSubtitle">Ручне підлаштування напруги за мультиметром</div>
    </div>
  </div>
  <div class="note-text" id="batteryCalCurrentLabel">Поточна напруга з сенсора: <strong id="batteryCalCurrentVoltage">--</strong> В</div>
  <div class="row" id="batteryCalMeasuredRow" style="margin-top: 12px;">
    <label for="batteryMeasuredVoltage" id="labelBatteryMeasuredVoltage">Фактична напруга з мультиметра (В):</label>
    <input type="number" id="batteryMeasuredVoltage" min="0" max="30" step="0.01" value="">
  </div>
  <div class="note-text" id="batteryCalNote" style="font-size: 12px; color: #6b7280;"></div>
  <div class="button-row" style="margin-top: 12px;">
    <button type="button" id="btnSaveBatteryCalibration" onclick="saveBatteryCalibration()">Застосувати калібрування</button>
  </div>
</div>

<div class="card auth-gated-card" style="display:none;">
  <div class="section-header">
    <div class="section-icon">
      <svg viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="10"></circle>
        <path d="M12 7v5"></path>
        <path d="M12 12l3 2"></path>
      </svg>
    </div>
    <div>
      <div class="section-title" id="timezoneSectionTitle">Часовий пояс</div>
      <div class="section-subtitle" id="timezoneSectionSubtitle">Окремо керуйте локальним часом пристрою</div>
    </div>
  </div>
  <div class="row" id="timezoneRow">
    <label for="timezoneOffsetMin" id="labelTimezoneOffset">Часовий пояс (UTC):</label>
    <select id="timezoneOffsetMin"></select>
  </div>
  <div class="note-text" id="wifiNoteTimezone" style="font-size: 12px; color: #6b7280;"></div>
  <div class="button-row" style="margin-top: 8px;">
    <button type="button" onclick="saveTimezone()" id="btnSaveTimezone">Зберегти часовий пояс</button>
  </div>
</div>

<script>
)rawliteral"
#include "shared/common_js_helpers.inc"
R"rawliteral(
let wifiLang = getStoredUiLang();
let apLoginRequired = false;
let apAuthenticated = false;
function wifiIsEn() { return wifiLang === 'en'; }
function formatTimezoneOffsetLabel(offsetMin) {
  const total = Number(offsetMin) || 0;
  const sign = total >= 0 ? '+' : '-';
  const abs = Math.abs(total);
  const hours = Math.floor(abs / 60);
  const mins = abs % 60;
  return `UTC${sign}${String(hours).padStart(2, '0')}:${String(mins).padStart(2, '0')}`;
}
function populateTimezoneOptions(selectedOffsetMin) {
  const select = document.getElementById('timezoneOffsetMin');
  if (!select) return;
  const current = Number.isFinite(Number(selectedOffsetMin)) ? Number(selectedOffsetMin) : Number(select.value || 120);
  select.innerHTML = '';
  for (let offset = -720; offset <= 840; offset += 15) {
    const option = document.createElement('option');
    option.value = String(offset);
    option.textContent = formatTimezoneOffsetLabel(offset);
    option.selected = offset === current;
    select.appendChild(option);
  }
}
function getSessionAwareHeaders(extraHeaders = {}) {
  const headers = { ...extraHeaders };
  const sessionToken = getApSessionToken();
  if (sessionToken) {
    headers['X-AquaFeed-Session'] = sessionToken;
  }
  return headers;
}
function applyApAuthUi() {
  const needsLogin = apLoginRequired && !apAuthenticated;
  const loginCard = document.getElementById('apLoginCard');
  const gatedCards = document.querySelectorAll('.auth-gated-card');
  const bottomTabs = document.querySelector('.bottom-tabs');
  if (loginCard) loginCard.style.display = needsLogin ? '' : 'none';
  gatedCards.forEach(card => {
    card.style.display = needsLogin ? 'none' : '';
  });
  if (bottomTabs) bottomTabs.style.display = needsLogin ? 'none' : 'flex';
}
function resetApSession() {
  setApSessionToken('');
  apAuthenticated = false;
  applyApAuthUi();
}
function showProtectedActionError(error, fallbackText) {
  const message = error && error.message ? error.message : '';
  if (message === 'login required') {
    resetApSession();
    showToast(wifiIsEn() ? 'Login required' : 'Спочатку увійдіть');
    updateStatus();
    return;
  }
  if (message === 'invalid password') {
    showToast(wifiIsEn() ? 'Wrong access point password' : 'Невірний пароль точки доступу');
    return;
  }
  showToast(fallbackText);
}
function loginAp() {
  const passwordInput = document.getElementById('apLoginPassword');
  const password = passwordInput ? passwordInput.value : '';
  if (!password || password.trim() === '') {
    showToast(wifiIsEn() ? 'Enter access point password' : 'Введіть пароль точки доступу');
    return;
  }
  postForm('/api/apLogin', { password })
    .then(expectOk)
    .then(r => r.json())
    .then(data => {
      setApSessionToken(data.token || '');
      apLoginRequired = !!data.requiresLogin;
      apAuthenticated = !!data.authenticated;
      if (passwordInput) passwordInput.value = '';
      applyApAuthUi();
      showToast(wifiIsEn() ? 'Access unlocked' : 'Доступ відкрито');
      updateStatus();
    })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'Login error' : 'Помилка входу'));
}
function applyWiFiLanguage() {
  document.title = wifiIsEn() ? 'WiFi settings' : 'Налаштування WiFi';
  const toastEl = document.getElementById('toast');
  if (toastEl) toastEl.textContent = wifiIsEn() ? 'Saved' : 'Збережено';
  const apLoginTitle = document.getElementById('apLoginTitle');
  if (apLoginTitle) apLoginTitle.textContent = wifiIsEn() ? 'Access login' : 'Підтвердження доступу';
  const apLoginSubtitle = document.getElementById('apLoginSubtitle');
  if (apLoginSubtitle) apLoginSubtitle.textContent = wifiIsEn()
    ? 'Enter the AquaFeed access point password to unlock settings'
    : 'Для зміни налаштувань введіть пароль точки доступу';
  const apLoginLabel = document.getElementById('apLoginLabel');
  if (apLoginLabel) apLoginLabel.textContent = wifiIsEn() ? 'Access point password:' : 'Пароль точки доступу:';
  const apLoginInput = document.getElementById('apLoginPassword');
  if (apLoginInput) apLoginInput.placeholder = wifiIsEn() ? 'Enter access point password' : 'Введіть пароль точки доступу';
  const apLoginButton = document.getElementById('btnApLogin');
  if (apLoginButton) apLoginButton.textContent = wifiIsEn() ? 'Unlock settings' : 'Увійти';
  const apLoginNote = document.getElementById('apLoginNote');
  if (apLoginNote) apLoginNote.innerHTML = wifiIsEn()
    ? 'Use the <strong>AquaFeed access point password</strong> to unlock local settings while the device is in AP mode.'
    : 'Використайте <strong>пароль точки доступу AquaFeed</strong>, щоб відкрити локальні налаштування в режимі AP.';
  const btnForget = document.getElementById('btnWifiForget');
  if (btnForget) btnForget.textContent = wifiIsEn() ? 'Forget network' : 'Забути мережу';
  const noteNet = document.getElementById('wifiNoteNetwork');
  if (noteNet) {
    noteNet.innerHTML = wifiIsEn()
      ? '<strong>Forget</strong> clears saved credentials and returns the device to AP mode at <code>http://192.168.4.1</code> or <code>http://fish.local</code>.'
      : 'Кнопка «Забути» видаляє збережені дані мережі та повертає пристрій у режим точки доступу за адресами <code>http://192.168.4.1</code> або <code>http://fish.local</code>.';
  }
  const wifiNetSub = document.getElementById('wifiNetworkSubtitle');
  if (wifiNetSub) wifiNetSub.textContent = wifiIsEn() ? 'Manage saved network' : 'Управління збереженою мережею';
  const lblPowerEco = document.getElementById('wifiLblPowerEco');
  if (lblPowerEco) lblPowerEco.textContent = wifiIsEn() ? 'Power saving mode' : 'Режим економії енергії';
  const lblDisp = document.getElementById('wifiLblDisplayToggle');
  if (lblDisp) lblDisp.textContent = wifiIsEn() ? 'Enable OLED display' : 'Увімкнути OLED дисплей';
  const noteOled = document.getElementById('wifiNoteOled');
  if (noteOled) noteOled.textContent = wifiIsEn()
    ? 'Turn off the display to save power. All features remain available in the web interface.'
    : 'Вимкніть дисплей для економії енергії. Всі функції працюють через веб-інтерфейс.';
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
  if (sectionSubtitles[0]) sectionSubtitles[0].textContent = wifiIsEn() ? 'Choose the web interface language' : 'Оберіть мову веб-інтерфейсу';
  if (sectionTitles[1]) sectionTitles[1].textContent = wifiIsEn() ? 'Connection status' : 'Статус підключення';
  if (sectionSubtitles[1]) sectionSubtitles[1].textContent = wifiIsEn() ? 'Current WiFi mode' : 'Поточний режим роботи WiFi';
  if (sectionTitles[3]) sectionTitles[3].textContent = wifiIsEn() ? 'Network settings' : 'Налаштування мережі';
  if (sectionSubtitles[3]) sectionSubtitles[3].textContent = wifiIsEn() ? 'Enter SSID and password to connect' : 'Введіть SSID та пароль для підключення';
  if (sectionTitles[4]) sectionTitles[4].textContent = wifiIsEn() ? 'Power settings' : 'Налаштування енергії';
  if (sectionSubtitles[4]) sectionSubtitles[4].textContent = wifiIsEn() ? 'Optimize power consumption' : 'Оптимізуйте споживання живлення';
  const timezoneSectionTitle = document.getElementById('timezoneSectionTitle');
  if (timezoneSectionTitle) timezoneSectionTitle.textContent = wifiIsEn() ? 'Time zone' : 'Часовий пояс';
  const timezoneSectionSubtitle = document.getElementById('timezoneSectionSubtitle');
  if (timezoneSectionSubtitle) timezoneSectionSubtitle.textContent = wifiIsEn()
    ? 'Manage the device local time separately'
    : 'Окремо керуйте локальним часом пристрою';
  const feedIntervalSectionTitle = document.getElementById('feedIntervalSectionTitle');
  if (feedIntervalSectionTitle) feedIntervalSectionTitle.textContent = wifiIsEn() ? 'Feeding interval' : 'Інтервал годування';
  const feedIntervalSectionSubtitle = document.getElementById('feedIntervalSectionSubtitle');
  if (feedIntervalSectionSubtitle) feedIntervalSectionSubtitle.textContent = wifiIsEn()
    ? 'Separate limit for how often feeding can run'
    : 'Окреме обмеження частоти запуску годувань';
  const batteryCalSectionTitle = document.getElementById('batteryCalSectionTitle');
  if (batteryCalSectionTitle) batteryCalSectionTitle.textContent = wifiIsEn() ? 'Battery calibration' : 'Калібрування батареї';
  const batteryCalSectionSubtitle = document.getElementById('batteryCalSectionSubtitle');
  if (batteryCalSectionSubtitle) batteryCalSectionSubtitle.textContent = wifiIsEn()
    ? 'Manual voltage adjustment using multimeter reading'
    : 'Ручне підлаштування напруги за мультиметром';
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
      ? '<strong>Power saving on:</strong> after the idle time below, the device enters <strong>deep sleep</strong> only when the pause is long enough. If the next feeding is soon, it stays awake. It wakes for the <strong>feed schedule</strong> (timer shortly before the next slot) or when you press the <strong>physical button</strong>. An open web page keeps it awake.'
      : '<strong>Режим економії увімкнено:</strong> після вказаного часу бездіяльності — <strong>глибокий сон</strong>, але лише якщо попереду справді довга пауза. Якщо наступне годування вже скоро, пристрій не засинає. Пробудження перед <strong>розкладом годування</strong> (таймер) або кнопкою на корпусі. Відкритий веб не дає заснути.';
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
  const labelFeedInterval = document.getElementById('labelFeedIntervalMin');
  if (labelFeedInterval) {
    labelFeedInterval.textContent = wifiIsEn()
      ? 'Minimum interval between feedings (min, 1-1440):'
      : 'Мінімальний інтервал між годуваннями (хв, 1–1440):';
  }
  const noteFeedInterval = document.getElementById('wifiNoteFeedInterval');
  if (noteFeedInterval) {
    noteFeedInterval.textContent = wifiIsEn()
      ? 'This limits how soon another manual or scheduled feed can start after the previous one. Default 5 min.'
      : 'Це обмежує, як швидко після попереднього можна запускати нове ручне або заплановане годування. За замовчуванням 5 хв.';
  }
  const btnFeedInterval = document.getElementById('btnSaveFeedInterval');
  if (btnFeedInterval) {
    btnFeedInterval.textContent = wifiIsEn() ? 'Save feeding interval' : 'Зберегти інтервал годування';
  }
  const batteryCalCurrentLabel = document.getElementById('batteryCalCurrentLabel');
  if (batteryCalCurrentLabel) {
    batteryCalCurrentLabel.innerHTML = wifiIsEn()
      ? 'Current sensor voltage: <strong id="batteryCalCurrentVoltage">--</strong> V'
      : 'Поточна напруга з сенсора: <strong id="batteryCalCurrentVoltage">--</strong> В';
  }
  const labelBatteryMeasured = document.getElementById('labelBatteryMeasuredVoltage');
  if (labelBatteryMeasured) {
    labelBatteryMeasured.textContent = wifiIsEn()
      ? 'Actual voltage from multimeter (V):'
      : 'Фактична напруга з мультиметра (В):';
  }
  const batteryCalNote = document.getElementById('batteryCalNote');
  if (batteryCalNote) {
    batteryCalNote.textContent = wifiIsEn()
      ? 'Enter the multimeter value and apply. The firmware computes and saves calibration automatically.'
      : 'Введіть значення з мультиметра і застосуйте. Прошивка автоматично перерахує та збереже калібрування.';
  }
  const btnBatteryCalibration = document.getElementById('btnSaveBatteryCalibration');
  if (btnBatteryCalibration) {
    btnBatteryCalibration.textContent = wifiIsEn() ? 'Apply calibration' : 'Застосувати калібрування';
  }
  const btnEnergy = document.getElementById('btnSaveEnergySettings');
  if (btnEnergy) {
    btnEnergy.textContent = wifiIsEn() ? 'Save power settings' : 'Зберегти налаштування економії енергії';
  }
  const labelTimezone = document.getElementById('labelTimezoneOffset');
  if (labelTimezone) {
    labelTimezone.textContent = wifiIsEn()
      ? 'Time zone (UTC offset):'
      : 'Часовий пояс (зсув UTC):';
  }
  const noteTimezone = document.getElementById('wifiNoteTimezone');
  if (noteTimezone) {
    noteTimezone.textContent = wifiIsEn()
      ? 'Select the device local UTC offset used for the clock, schedule, and next feeding countdown. If your region changes daylight saving time, update this manually.'
      : 'Оберіть локальний зсув UTC для годинника, розкладу та відліку до наступного годування. Якщо у вашому регіоні змінюється літній/зимовий час, оновлюйте це вручну.';
  }
  const btnTimezone = document.getElementById('btnSaveTimezone');
  if (btnTimezone) {
    btnTimezone.textContent = wifiIsEn() ? 'Save time zone' : 'Зберегти часовий пояс';
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
  showToastMessage(text);
}

function forgetWiFi(){
  showToast(wifiIsEn() ? 'Removing network...' : 'Видаляю мережу...');
  postForm('/api/forgetWiFi')
    .then(expectOk)
    .then(()=>{
      showToast(wifiIsEn() ? 'Network forgotten. Reconnect to the AquaFeed access point' : 'Мережу забуто. Повторно підключіться до точки доступу AquaFeed');
      setTimeout(()=>{ window.location.reload(); }, 2000);
    })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'WiFi remove error' : 'Помилка видалення WiFi'));
}

function saveEnergySettings(){
  const powerModeEl = document.getElementById('powerSaveMode');
  const displayToggleEl = document.getElementById('displayEnabled');
  const powerEnabled = !!(powerModeEl && powerModeEl.checked);
  const displayEnabled = !!(displayToggleEl && displayToggleEl.checked);

  const idleEl = document.getElementById('deepSleepIdleSec');
  let idleSec = idleEl ? parseInt(idleEl.value, 10) : 300;
  if (!Number.isFinite(idleSec)) idleSec = 300;
  idleSec = Math.max(10, Math.min(3600, idleSec));
  if (idleEl) idleEl.value = idleSec;

  const displayEl = document.getElementById('displayOffSec');
  let displaySec = displayEl ? parseInt(displayEl.value, 10) : 20;
  if (!Number.isFinite(displaySec)) displaySec = 20;
  displaySec = Math.max(5, Math.min(600, displaySec));
  if (displayEl) displayEl.value = displaySec;

  Promise.all([
    postForm('/api/setPowerMode', { enabled: powerEnabled ? 'true' : 'false', idleSec }).then(expectOk),
    postForm('/api/setDisplayMode', { enabled: displayEnabled ? 'true' : 'false', sec: displaySec }).then(expectOk),
  ])
    .then(() => {
      showToast(wifiIsEn() ? 'Saved' : 'Збережено');
      updateStatus();
    })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'Save error' : 'Помилка збереження'));
}

function saveFeedInterval(){
  const feedIntervalEl = document.getElementById('feedIntervalMin');
  let feedIntervalMinutes = feedIntervalEl ? parseInt(feedIntervalEl.value, 10) : 5;
  if (!Number.isFinite(feedIntervalMinutes)) feedIntervalMinutes = 5;
  feedIntervalMinutes = Math.max(1, Math.min(1440, feedIntervalMinutes));
  if (feedIntervalEl) feedIntervalEl.value = feedIntervalMinutes;

  postForm('/api/setFeedInterval', { minutes: feedIntervalMinutes })
    .then(expectOk)
    .then(() => {
      showToast(wifiIsEn() ? 'Saved' : 'Збережено');
      updateStatus();
    })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'Save error' : 'Помилка збереження'));
}

function saveBatteryCalibration(){
  const measuredEl = document.getElementById('batteryMeasuredVoltage');
  let measuredVoltage = measuredEl ? parseFloat(measuredEl.value) : NaN;
  if (!Number.isFinite(measuredVoltage)) {
    showToast(wifiIsEn() ? 'Enter multimeter voltage' : 'Введіть напругу з мультиметра');
    return;
  }
  measuredVoltage = Math.max(0.1, Math.min(30.0, measuredVoltage));
  if (measuredEl) measuredEl.value = measuredVoltage.toFixed(2);

  postForm('/api/setBatteryCalibration', { measuredVoltage: measuredVoltage.toFixed(3) })
    .then(expectOk)
    .then(() => {
      showToast(wifiIsEn() ? 'Battery calibration saved' : 'Калібрування батареї збережено');
      updateStatus();
    })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'Calibration save error' : 'Помилка збереження калібрування'));
}

function saveTimezone(){
  const el = document.getElementById('timezoneOffsetMin');
  let offsetMin = el ? parseInt(el.value, 10) : 120;
  if (!Number.isFinite(offsetMin)) offsetMin = 120;
  offsetMin = Math.max(-720, Math.min(840, offsetMin));
  if (el) el.value = String(offsetMin);
  postForm('/api/setTimezone', { offsetMin })
    .then(expectOk)
    .then(()=>{ showToast(wifiIsEn() ? 'Saved' : 'Збережено'); updateStatus(); })
    .catch(error => showProtectedActionError(error, wifiIsEn() ? 'Save error' : 'Помилка збереження'));
}

function updateStatus(){
  Promise.all([
    fetch('/api/status', { headers: getSessionAwareHeaders() }).then(expectOk).then(r => r.json()),
    fetch('/api/apLoginStatus', { headers: getSessionAwareHeaders({ 'X-AquaFeed-Client': 'webui' }) })
      .then(expectOk)
      .then(r => r.json())
      .catch(() => null),
  ]).then(([j, auth])=>{
    apLoginRequired = auth ? !!auth.requiresLogin : !!j.isAPMode;
    apAuthenticated = auth ? !!auth.authenticated : !j.isAPMode;
    if (apLoginRequired && !apAuthenticated) {
      setApSessionToken('');
    }
    applyApAuthUi();
    const statusText = document.getElementById('wifiStatusText');
    const statusPill = document.getElementById('wifiStatusPill');
    if (statusPill) {
      statusPill.classList.remove('success','warning','error');
    }
    statusText.style.color = '';
    if(j.isAPMode) {
      statusText.innerText = (wifiIsEn() ? 'Access Point mode (AP) - ' : 'Режим точки доступу (AP) - ') + (j.wifiSSID || (wifiIsEn() ? 'not set' : 'не налаштовано'));
      if (statusPill) statusPill.classList.add('warning');
    } else if(j.wifiIP) {
      statusText.innerText = (wifiIsEn() ? 'Connected to: ' : 'Підключено до: ') + (j.wifiSSID || (wifiIsEn() ? 'unknown' : 'невідомо')) + ' (IP: ' + j.wifiIP + ')';
      if (statusPill) statusPill.classList.add('success');
    } else {
      statusText.innerText = wifiIsEn() ? 'Not connected' : 'Не підключено';
      if (statusPill) statusPill.classList.add('error');
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
    const feedIntervalEl = document.getElementById('feedIntervalMin');
    if (feedIntervalEl && typeof j.minFeedIntervalMin === 'number') {
      feedIntervalEl.value = j.minFeedIntervalMin;
    }
    const batteryCalCurrentEl = document.getElementById('batteryCalCurrentVoltage');
    if (batteryCalCurrentEl) {
      if (typeof j.batteryVoltage === 'number' && Number.isFinite(j.batteryVoltage)) {
        batteryCalCurrentEl.textContent = j.batteryVoltage.toFixed(2);
      } else {
        batteryCalCurrentEl.textContent = '--';
      }
    }
    const batteryMeasuredEl = document.getElementById('batteryMeasuredVoltage');
    if (batteryMeasuredEl && (!batteryMeasuredEl.value || batteryMeasuredEl.value.trim() === '')) {
      if (typeof j.batteryVoltage === 'number' && Number.isFinite(j.batteryVoltage)) {
        batteryMeasuredEl.value = j.batteryVoltage.toFixed(2);
      }
    }
    populateTimezoneOptions(typeof j.timezoneOffsetMin === 'number' ? j.timezoneOffsetMin : 120);
    syncPowerEcoUiFromCheckbox();
  }).catch(() => {
    showToast(wifiIsEn() ? 'Status load error' : 'Помилка завантаження статусу');
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
  setApSessionToken('');
  applyApAuthUi();
  const ukBtn = document.getElementById('wifiLangUkBtn');
  const enBtn = document.getElementById('wifiLangEnBtn');
  if (ukBtn) ukBtn.addEventListener('click', () => setWiFiLanguage('uk'));
  if (enBtn) enBtn.addEventListener('click', () => setWiFiLanguage('en'));
  const apLoginInput = document.getElementById('apLoginPassword');
  if (apLoginInput) {
    apLoginInput.addEventListener('keydown', (event) => {
      if (event.key === 'Enter') loginAp();
    });
  }
  const ps = document.getElementById('powerSaveMode');
  if (ps) ps.addEventListener('change', syncPowerEcoUiFromCheckbox);
  populateTimezoneOptions(120);
  applyWiFiLanguage();
  updateStatus();
};
document.addEventListener('DOMContentLoaded', function() {
  setActiveBottomTab();
});
</script>

)rawliteral"
#include "shared/bottom_tabs_wifi.inc"
R"rawliteral(
</body>
</html>
)rawliteral";
