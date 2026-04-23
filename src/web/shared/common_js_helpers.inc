R"rawliteral(
function getStoredUiLang() {
  return localStorage.getItem('aqua_lang') || 'uk';
}

function isStoredUiLangEn() {
  return getStoredUiLang() === 'en';
}

function showToastMessage(text) {
  const toast = document.getElementById('toast');
  if (!toast) return;
  toast.innerText = text;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2000);
}

function setActiveBottomTab(options = {}) {
  const currentPath = options.currentPath || window.location.pathname;
  const matchRootToHome = !!options.matchRootToHome;
  const tabs = document.querySelectorAll('.bottom-tab');
  tabs.forEach(tab => {
    const href = tab.getAttribute('href');
    const isActive = href === currentPath || (matchRootToHome && currentPath === '/' && href === '/');
    tab.classList.toggle('active', isActive);
  });
}

function getApSessionToken() {
  try {
    return sessionStorage.getItem('aquafeed_ap_session') || '';
  } catch (_) {
    return '';
  }
}

function setApSessionToken(token) {
  try {
    if (token) sessionStorage.setItem('aquafeed_ap_session', token);
    else sessionStorage.removeItem('aquafeed_ap_session');
  } catch (_) {}
}

function postForm(path, values = {}) {
  const body = new URLSearchParams();
  Object.entries(values).forEach(([key, value]) => {
    if (value !== undefined && value !== null) {
      body.append(key, String(value));
    }
  });
  const headers = {
    'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8',
    'X-AquaFeed-Client': 'webui',
  };
  const sessionToken = getApSessionToken();
  if (sessionToken) {
    headers['X-AquaFeed-Session'] = sessionToken;
  }
  return fetch(path, {
    method: 'POST',
    headers,
    body: body.toString(),
  });
}

function expectOk(response) {
  if (response.ok) return response;
  return response.text().then(text => Promise.reject(new Error(text || ('HTTP ' + response.status))));
}

function voltageToPercentClient(v) {
  const MAX_VOLTAGE = 8.4;
  const MIN_VOLTAGE = 6.6;
  if (!Number.isFinite(v)) return null;
  if (v >= MAX_VOLTAGE) return 100;
  if (v <= MIN_VOLTAGE) return 0;
  return Math.round(((v - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100);
}
)rawliteral"
