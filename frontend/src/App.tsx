import { useCallback, useEffect, useRef, useState } from 'react'
import { api, authStorage } from './services/api'
import { LoginPage } from './components/LoginPage'
import type { FeedTime, StatusResponse } from './types'
import { APP_VERSION } from './version'
import { TRANSLATIONS, type Lang } from './translations'
import './App.css'

type Tab = 'home' | 'info' | 'settings'

function formatBytes(bytes: number): string {
  if (bytes >= 1024) return (bytes / 1024).toFixed(2) + ' KB'
  return bytes + ' B'
}

interface LocalFeedTime extends FeedTime {
  day: string
}

function BatteryGauge({ percent, isCharging }: { percent: number; isCharging: boolean }) {
  const total = 345.58
  const offset = total * (1 - Math.max(0, Math.min(100, percent)) / 100)
  const color = isCharging ? '#4CAF50' : percent > 50 ? '#4CAF50' : percent > 20 ? '#FF9800' : '#f44336'
  return (
    <svg className="aq-gauge-svg" viewBox="0 0 260 160"
      style={{ '--charge-full': total, '--charge-target': offset } as React.CSSProperties}>
      <path d="M 20 140 A 110 110 0 0 1 240 140" fill="none" stroke="#E6E9EF" strokeWidth="14" strokeLinecap="round" />
      <path d="M 20 140 A 110 110 0 0 1 240 140" fill="none" stroke={color} strokeWidth="14" strokeLinecap="round"
        strokeDasharray={total} strokeDashoffset={offset}
        className={isCharging ? 'aq-gauge-fill-arc aq-gauge-charging-arc' : 'aq-gauge-fill-arc'} />
      <text x="130" y="125" textAnchor="middle" dominantBaseline="middle" fontSize="28" fontWeight="700" fill="#222">{percent}%</text>
    </svg>
  )
}

function NextFeedGauge({ minutes, label }: { minutes?: number | null; label: string }) {
  const total = 345.58
  const ratio = minutes != null ? Math.max(0, Math.min(1440, minutes)) / 1440 : 0
  const offset = total * ratio
  return (
    <svg className="aq-gauge-svg" viewBox="0 0 260 160">
      <path d="M 20 140 A 110 110 0 0 1 240 140" fill="none" stroke="#E6E9EF" strokeWidth="14" strokeLinecap="round" />
      <path d="M 20 140 A 110 110 0 0 1 240 140" fill="none" stroke="#1976D2" strokeWidth="14" strokeLinecap="round"
        strokeDasharray={total} strokeDashoffset={offset} className="aq-gauge-fill-arc" />
      <text x="130" y="118" textAnchor="middle" dominantBaseline="middle" fontSize="17" fontWeight="600" fill="#1976D2">{label}</text>
    </svg>
  )
}

function App() {
  const [token, setToken] = useState<string | null>(authStorage.getToken)
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [tab, setTab] = useState<Tab>('home')
  const [feeding, setFeeding] = useState(false)
  const [feedRepeats, setFeedRepeats] = useState(1)
  const [localSchedule, setLocalSchedule] = useState<LocalFeedTime[]>([])
  const [angle, setAngle] = useState(90)
  const [speed, setSpeed] = useState(50)
  const [toastMsg, setToastMsg] = useState('')
  const [toastVisible, setToastVisible] = useState(false)
  const toastTimer = useRef<ReturnType<typeof setTimeout> | null>(null)
  const scheduleEdited = useRef(false)

  // Language (app-level)
  const [lang, setLang] = useState<Lang>(() => (localStorage.getItem('aq_lang') as Lang) || 'uk')
  const T = TRANSLATIONS[lang]

  // Settings form state
  const [sDeepSleep, setSDeepSleep] = useState(120)
  const [sDisplayEnabled, setSDisplayEnabled] = useState(true)
  const [sDisplayOff, setSDisplayOff] = useState(30)
  const [sMinInterval, setSMinInterval] = useState(5)
  const [sCalibVoltage, setSCalibVoltage] = useState('')
  const [sTimezone, setSTimezone] = useState(2)
  const settingsInited = useRef(false)

  // Food supply tracking (localStorage-based)
  const [foodTotalG, setFoodTotalG] = useState(() => parseFloat(localStorage.getItem('aq_food_total') || '0'))
  const [foodLoadedTs, setFoodLoadedTs] = useState(() => parseInt(localStorage.getItem('aq_food_ts') || '0'))
  const [gramsPerFeed, setGramsPerFeed] = useState(() => parseFloat(localStorage.getItem('aq_food_gpf') || '0'))
  const [foodFormOpen, setFoodFormOpen] = useState(false)
  const [foodInputG, setFoodInputG] = useState('')
  const [foodInputGpf, setFoodInputGpf] = useState('')

  const showToast = (msg: string) => {
    if (toastTimer.current) clearTimeout(toastTimer.current)
    setToastMsg(msg)
    setToastVisible(true)
    toastTimer.current = setTimeout(() => setToastVisible(false), 2600)
  }

  const fetchStatus = useCallback(async () => {
    if (!authStorage.getToken()) return
    try {
      const data = await api.getStatus()
      setStatus(data)
      if (!scheduleEdited.current) {
        setLocalSchedule(data.feedTimes.map(ft => ({ ...ft, day: T.days[0] })))
      }
      setAngle(data.angle)
      setSpeed(data.speed)
      setFeedRepeats(r => r === 1 ? data.feedRepeats : r)
      if (!settingsInited.current) {
        setSDeepSleep(data.deepSleepIdleSec)
        setSDisplayEnabled(data.displayEnabled)
        setSDisplayOff(data.displayOffAfterSec)
        setSMinInterval(data.minFeedIntervalMin)
        settingsInited.current = true
      }
    } catch {
      // silent on polling errors
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    if (!token) { setLoading(false); return }
    fetchStatus()
    const iv = setInterval(fetchStatus, 5000)
    return () => clearInterval(iv)
  }, [token, fetchStatus])

  const handleLoginSuccess = (accessToken: string, refreshToken: string) => {
    authStorage.save(accessToken, refreshToken)
    setToken(accessToken)
    setLoading(true)
  }

  const handleLogout = () => {
    authStorage.clear()
    setToken(null)
    setStatus(null)
    setLoading(true)
  }

  const handleFeed = async () => {
    if (feeding) return
    setFeeding(true)
    showToast(T.toastFeeding)
    try {
      await api.feedNow({ repeats: feedRepeats })
      setTimeout(() => { fetchStatus(); setFeeding(false); showToast(T.toastFed) }, 1000)
    } catch {
      setFeeding(false)
      showToast(T.toastFeedError)
    }
  }

  const handleSaveSchedule = async () => {
    try {
      await api.setSchedule({ times: localSchedule.map(({ day: _day, ...ft }) => ft) })
      scheduleEdited.current = false
      showToast(T.toastScheduleSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const handleSaveControl = async () => {
    try {
      await Promise.all([api.setAngle({ angle }), api.setSpeed({ speed })])
      showToast(T.toastSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const addFeedTime = () => {
    setLocalSchedule(prev => [...prev, { hour: 12, minute: 0, repeats: 1, day: T.days[0] }])
    scheduleEdited.current = true
    showToast(T.toastFeedAdded)
  }

  const removeFeedTime = (i: number) => {
    setLocalSchedule(prev => prev.filter((_, idx) => idx !== i))
    scheduleEdited.current = true
    showToast(T.toastFeedRemoved)
  }

  const updateFeedTimeHM = (i: number, h: number, m: number) => {
    setLocalSchedule(prev => prev.map((ft, idx) => idx === i ? { ...ft, hour: h, minute: m } : ft))
    scheduleEdited.current = true
  }

  const updateFeedRepeats = (i: number, r: number) => {
    setLocalSchedule(prev => prev.map((ft, idx) => idx === i ? { ...ft, repeats: r } : ft))
    scheduleEdited.current = true
  }

  const updateFeedDay = (i: number, day: string) => {
    setLocalSchedule(prev => prev.map((ft, idx) => idx === i ? { ...ft, day } : ft))
    scheduleEdited.current = true
  }

  // Food supply computed values
  const feedingsPerDay = localSchedule.length
  const msPerDay = 86_400_000
  const daysElapsed = foodLoadedTs > 0 ? (Date.now() - foodLoadedTs) / msPerDay : 0
  const foodConsumed = daysElapsed * feedingsPerDay * gramsPerFeed
  const foodRemaining = Math.max(0, foodTotalG - foodConsumed)
  const foodPercent = foodTotalG > 0 ? (foodRemaining / foodTotalG) * 100 : 0
  const foodDaysLeft = feedingsPerDay > 0 && gramsPerFeed > 0
    ? foodRemaining / (feedingsPerDay * gramsPerFeed)
    : 0
  const foodMonths = Math.floor(foodDaysLeft / 30)
  const foodDaysRem = Math.floor(foodDaysLeft % 30)

  const handleFoodSave = () => {
    const g = parseFloat(foodInputG)
    const gpf = parseFloat(foodInputGpf)
    if (!g || g <= 0 || !gpf || gpf <= 0) return
    const ts = Date.now()
    localStorage.setItem('aq_food_total', String(g))
    localStorage.setItem('aq_food_ts', String(ts))
    localStorage.setItem('aq_food_gpf', String(gpf))
    setFoodTotalG(g)
    setFoodLoadedTs(ts)
    setGramsPerFeed(gpf)
    setFoodFormOpen(false)
  }

  if (!token) return <LoginPage onSuccess={handleLoginSuccess} />

  if (loading) {
    return (
      <div className="aq-shell">
        <div className="aq-loading">
          <div className="aq-spinner" />
          <div className="aq-loading-text">{T.connecting}</div>
        </div>
      </div>
    )
  }

  const cooldown = status?.manualFeedCooldownSeconds ?? 0

  return (
    <div className="aq-shell">
      <div className={`aq-toast ${toastVisible ? 'show' : ''}`}>
        <span className="aq-toast-icon">i</span>
        <span>{toastMsg}</span>
      </div>

      <div className="aq-content">
        {/* Header */}
        <div className="aq-hero">
          <div className="app-illustration">
            <svg className="hero-svg-fish" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg">
              <path className="hero-fish-body" d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4" fill="#728389"/>
              <g fill="#8d9ba3">
                <path className="hero-fish-tail" d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"/>
                <ellipse cx="33.4" cy="35.3" rx="2.2" ry="3.2"/>
                <ellipse cx="37.6" cy="39.2" rx="1.2" ry="2.5"/>
                <ellipse cx="39.9" cy="36" rx=".6" ry="1.7"/>
              </g>
              <g fill="#75d6ff">
                <ellipse className="hero-bubble hero-bubble-1" cx="5.3" cy="44" rx="1.7" ry="1.8"/>
                <ellipse className="hero-bubble hero-bubble-2" cx="6.3" cy="23.4" rx="4.3" ry="4.5"/>
                <ellipse className="hero-bubble hero-bubble-3" cx="12.8" cy="10.3" rx="8" ry="8.3"/>
              </g>
              <ellipse cx="18.7" cy="38.5" rx="7.1" ry="7.4" fill="#fcfcfa"/>
              <ellipse className="hero-fish-eye" cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c"/>
            </svg>
          </div>
          <div className="aq-app-heading">
            <div className="aq-app-title">{T.appTitle}</div>
            <div className="aq-app-subtitle">{T.appSubtitle}</div>
          </div>
        </div>

        {/* ── HOME TAB ── */}
        {tab === 'home' && status && (
          <>
            {/* Deep sleep help banner */}
            <div className="aq-info-banner">
              {T.deepSleepBanner}
            </div>

            {/* Battery + Next Feed */}
            <div className="aq-card aq-battery-card">
              <div className="aq-gauges-row">
                <div className="aq-gauge-wrap">
                  <BatteryGauge percent={status.batteryPercent} isCharging={status.isCharging} />
                  <div className="aq-gauge-title">{T.batteryStatus}</div>
                  <div className="aq-gauge-sub">{T.voltageLabel} {status.batteryVoltage.toFixed(2)} V</div>
                </div>
                <div className="aq-gauge-wrap">
                  <NextFeedGauge minutes={status.nextFeedMinutes} label={status.nextFeedMinutes != null ? T.nextFeedIn(Math.floor(status.nextFeedMinutes / 60), status.nextFeedMinutes % 60) : '—'} />
                  <div className="aq-gauge-title">{T.untilNextFeed}</div>
                </div>
              </div>
              {status.isCharging && (
                <div className="aq-alert aq-alert-charging"><span>i</span> {T.isCharging}</div>
              )}
              {!status.isCharging && status.batteryPercent <= 20 && (
                <div className="aq-alert aq-alert-low"><span>!</span> {T.lowBattery}</div>
              )}

              {/* Food supply tracker */}
              <div className="aq-food-section">
                <div className="aq-food-header">
                  <span className="aq-food-title">{T.foodTitle}</span>
                  <button
                    className="aq-food-refill-btn"
                    onClick={() => {
                      setFoodInputG(foodTotalG > 0 ? String(foodTotalG) : '')
                      setFoodInputGpf(gramsPerFeed > 0 ? String(gramsPerFeed) : '')
                      setFoodFormOpen(v => !v)
                    }}
                  >{T.foodRefill}</button>
                </div>

                {foodTotalG > 0 && gramsPerFeed > 0 && feedingsPerDay > 0 ? (
                  <>
                    <div className="aq-food-bar-track">
                      <div
                        className="aq-food-bar-fill"
                        style={{
                          width: `${foodPercent}%`,
                          background: foodPercent > 40 ? '#4CAF50' : foodPercent > 15 ? '#FF9800' : '#f44336'
                        }}
                      />
                    </div>
                    <div className="aq-food-meta">
                      <span>{T.foodRemaining(Math.round(foodRemaining))} • {T.foodDuration(foodMonths, foodDaysRem)}</span>
                      <span className="aq-food-meta-right">{T.foodFeedingsPerDay(feedingsPerDay)}</span>
                    </div>
                  </>
                ) : (
                  <div className="aq-food-hint">
                    {feedingsPerDay === 0 ? T.foodNoSchedule : T.foodNotSet}
                  </div>
                )}

                {foodFormOpen && (
                  <div className="aq-food-form">
                    <div className="aq-food-form-row">
                      <label className="aq-food-form-label">{T.foodGramsTotal}</label>
                      <input
                        className="aq-food-form-input"
                        type="number"
                        min="1"
                        max="9999"
                        step="1"
                        value={foodInputG}
                        onChange={e => setFoodInputG(e.target.value)}
                        placeholder="100"
                      />
                    </div>
                    <div className="aq-food-form-row">
                      <label className="aq-food-form-label">{T.foodGramsPerFeed}</label>
                      <input
                        className="aq-food-form-input"
                        type="number"
                        min="0.1"
                        max="100"
                        step="0.1"
                        value={foodInputGpf}
                        onChange={e => setFoodInputGpf(e.target.value)}
                        placeholder="0.5"
                      />
                    </div>
                    <div className="aq-food-form-btns">
                      <button className="aq-food-save-btn" onClick={handleFoodSave}>{T.foodSave}</button>
                      <button className="aq-food-cancel-btn" onClick={() => setFoodFormOpen(false)}>{T.foodCancel}</button>
                    </div>
                  </div>
                )}
              </div>
            </div>

            {/* Manual Feed */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M5 11h14"/><path d="M7 11v2a5 5 0 0 0 10 0v-2"/>
                    <path d="M9 6.5l1.2 3"/><path d="M15 6.5l-1.2 3"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.manualFeed}</div>
                  <div className="aq-section-sub">{T.manualFeedSub}</div>
                </div>
              </div>
              <div className="aq-repeats-row">
                <span className="aq-repeats-label">{T.repeatCount}</span>
                <div className="aq-stepper">
                  <button className="aq-stepper-btn" onClick={() => setFeedRepeats(r => Math.max(1, r - 1))}>−</button>
                  <span className="aq-stepper-val">{feedRepeats}</span>
                  <button className="aq-stepper-btn" onClick={() => setFeedRepeats(r => Math.min(10, r + 1))}>+</button>
                </div>
              </div>
              <button
                className={`aq-feed-btn ${feeding ? 'is-feeding' : ''}`}
                onClick={handleFeed}
                disabled={feeding || cooldown > 0}
              >
                {feeding
                  ? T.feeding
                  : cooldown > 0
                    ? T.feedNowCooldown(cooldown)
                    : T.feedNow}
              </button>
            </div>

            {/* Schedule */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="7"/>
                    <line x1="12" y1="12" x2="12" y2="7.5"/>
                    <line x1="12" y1="12" x2="15.5" y2="13.5"/>
                    <path d="M4.5 5.5l1.5 1.5"/><path d="M19.5 5.5l-1.5 1.5"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.schedule}</div>
                  <div className="aq-section-sub">{T.scheduleSub}</div>
                  {status.nextFeedMinutes != null && (
                    <div className="aq-section-sub">
                      {T.nextFeedLabel(Math.floor(status.nextFeedMinutes / 60), status.nextFeedMinutes % 60)}
                    </div>
                  )}
                </div>
              </div>
              <div className="aq-schedule-list">
                {localSchedule.map((ft, i) => (
                  <div key={i} className="aq-feed-block">
                    <div className="aq-feed-time-col">
                      <input
                        type="time"
                        className="aq-time-input"
                        value={`${String(ft.hour).padStart(2, '0')}:${String(ft.minute).padStart(2, '0')}`}
                        onChange={e => {
                          const [h, m] = e.target.value.split(':').map(Number)
                          updateFeedTimeHM(i, h, m)
                        }}
                      />
                    </div>
                    <div className="aq-feed-center">
                      <div className="aq-feed-field">
                        <span>{T.repeats}</span>
                        <div className="aq-stepper aq-stepper-sm">
                          <button className="aq-stepper-btn aq-stepper-btn-sm" onClick={() => updateFeedRepeats(i, Math.max(1, ft.repeats - 1))}>−</button>
                          <span className="aq-stepper-val">{ft.repeats}</span>
                          <button className="aq-stepper-btn aq-stepper-btn-sm" onClick={() => updateFeedRepeats(i, Math.min(10, ft.repeats + 1))}>+</button>
                        </div>
                      </div>
                      <div className="aq-feed-field">
                        <span>{T.day}</span>
                        <select className="aq-day-select" value={ft.day} onChange={e => updateFeedDay(i, e.target.value)}>
                          {T.days.map(d => <option key={d} value={d}>{d}</option>)}
                        </select>
                      </div>
                    </div>
                    <button className="aq-remove-btn" onClick={() => removeFeedTime(i)}>×</button>
                  </div>
                ))}
              </div>
              <button className="aq-add-btn" onClick={addFeedTime}>{T.addFeeding}</button>
              <button className="aq-save-btn" onClick={handleSaveSchedule}>{T.saveAllTimes}</button>
            </div>

            {/* Manual Control */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <line x1="8" y1="5" x2="8" y2="19"/><line x1="16" y1="5" x2="16" y2="19"/>
                    <circle cx="8" cy="10" r="2.5"/><circle cx="16" cy="14" r="2.5"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.manualControl}</div>
                  <div className="aq-section-sub">{T.manualControlSub}</div>
                </div>
              </div>
              <div className="aq-slider-row">
                <div className="aq-slider-label">
                  <span>{T.servoAngle(angle)}</span>
                </div>
                <input type="range" className="aq-slider" min="0" max="180" value={angle} onChange={e => setAngle(Number(e.target.value))} />
              </div>
              <div className="aq-slider-row">
                <div className="aq-slider-label">
                  <span>{T.servoSpeed(speed)}</span>
                </div>
                <input type="range" className="aq-slider" min="1" max="100" value={speed} onChange={e => setSpeed(Number(e.target.value))} />
              </div>
              <button className="aq-save-btn" onClick={handleSaveControl}>{T.saveSpeed}</button>
            </div>
          </>
        )}

        {/* ── INFO TAB ── */}
        {tab === 'info' && status && (
          <>
            {/* WiFi */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M4 9c4.5-4.5 11.5-4.5 16 0"/>
                    <path d="M7 12c2.8-2.8 7.2-2.8 10 0"/>
                    <path d="M10.5 15.5c1-1 3-1 4 0"/>
                    <circle cx="12" cy="19" r="1.2" fill="#4A5568" stroke="none"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.wifiInfo}</div>
                  <div className="aq-section-sub">{T.wifiInfoSub}</div>
                </div>
              </div>
              <div className="aq-info-row"><span>{T.ssid}</span><span>{status.wifiSSID || T.notConfigured}</span></div>
              <div className="aq-info-row"><span>{T.ipAddr}</span><span className="aq-mono">{status.wifiIP || T.notConnected}</span></div>
              <div className="aq-info-row"><span>{T.modeLabel}</span><span>{status.isAPMode ? T.apMode : T.staMode}</span></div>
              <div className="aq-info-row"><span>{T.mdns}</span><span className="aq-mono">fish.local</span></div>
            </div>

            {/* Батарея */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <rect x="5" y="8" width="12" height="8" rx="2"/>
                    <path d="M17 11h2.2a1 1 0 0 1 0 2H17"/>
                    <rect x="7.5" y="10" width="5" height="4" rx="1" fill="#4A5568" stroke="none"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.batterySection}</div>
                  <div className="aq-section-sub">{T.batterySectionSub}</div>
                </div>
              </div>
              <div className="aq-info-row"><span>{T.voltageValue}</span><span>{status.batteryVoltage.toFixed(2)} В</span></div>
              <div className="aq-info-row"><span>{T.percentValue}</span><span>{status.batteryPercent}%</span></div>
            </div>

            {/* Налаштування */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="3.5"/>
                    <path d="M12 3v2"/><path d="M12 19v2"/>
                    <path d="M21 12h-2"/><path d="M5 12H3"/>
                    <path d="M18.5 6l-1.4 1.4"/><path d="M6.9 17.6 5.5 19"/>
                    <path d="M18.5 18.5 17.1 17.1"/><path d="M6.9 6.9 5.5 5.5"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.settingsInfo}</div>
                  <div className="aq-section-sub">{T.settingsInfoSub}</div>
                </div>
              </div>
              <div className="aq-info-row"><span>{T.servoSpeedLabel}</span><span>{status.speed}</span></div>
              <div className="aq-info-row"><span>{T.feedRepeatsLabel}</span><span>{status.feedRepeats}</span></div>
              <div className="aq-info-row"><span>{T.powerSaveLabel}</span><span>{status.powerSaveMode ? T.on : T.off}</span></div>
              <div className="aq-info-row"><span>{T.oledLabel}</span><span>{status.displayEnabled ? T.on : T.off}</span></div>
              <div className="aq-info-row"><span>{T.scheduleCountLabel}</span><span>{status.feedTimes.length}</span></div>
            </div>

            {/* Система */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <rect x="7" y="4" width="10" height="16" rx="2"/>
                    <line x1="9" y1="8" x2="15" y2="8"/>
                    <line x1="10" y1="18" x2="14" y2="18"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.systemInfo}</div>
                  <div className="aq-section-sub">{T.systemInfoSub}</div>
                </div>
              </div>
              <div className="aq-info-row"><span>{T.model}</span><span>AquaFeed Hub</span></div>
              <div className="aq-info-row"><span>{T.firmwareVersion}</span><span className="aq-mono">{APP_VERSION}</span></div>
              {status.uptimeSeconds != null && (
                <div className="aq-info-row">
                  <span>{T.uptime}</span>
                  <span>{T.uptimeFmt(Math.floor(status.uptimeSeconds / 3600), Math.floor((status.uptimeSeconds % 3600) / 60))}</span>
                </div>
              )}
              {status.cpuFrequency != null && (
                <div className="aq-info-row"><span>{T.cpuFreq}</span><span>{status.cpuFrequency} MHz</span></div>
              )}
              {status.currentTime && (
                <div className="aq-info-row"><span>{T.deviceTime}</span><span className="aq-mono">{status.currentTime}</span></div>
              )}
            </div>

            {/* Пам'ять та кеш */}
            {status.memoryFreeHeap != null && (
              <div className="aq-card">
                <div className="aq-section-header">
                  <div className="aq-section-icon">
                    <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                      <rect x="3" y="3" width="18" height="18" rx="2" ry="2"/>
                      <line x1="9" y1="9" x2="15" y2="9"/>
                      <line x1="9" y1="15" x2="15" y2="15"/>
                    </svg>
                  </div>
                  <div>
                    <div className="aq-section-title">{T.memoryInfo}</div>
                    <div className="aq-section-sub">{T.memoryInfoSub}</div>
                  </div>
                </div>
                <div className="aq-info-row"><span>{T.freeMemory}</span><span>{formatBytes(status.memoryFreeHeap)}</span></div>
                {status.memoryUsedHeap != null && <div className="aq-info-row"><span>{T.usedMemory}</span><span>{formatBytes(status.memoryUsedHeap)}</span></div>}
                {status.memoryTotalHeap != null && <div className="aq-info-row"><span>{T.totalMemory}</span><span>{formatBytes(status.memoryTotalHeap)}</span></div>}
                {status.memoryMaxAllocHeap != null && <div className="aq-info-row"><span>{T.maxBlock}</span><span>{formatBytes(status.memoryMaxAllocHeap)}</span></div>}
                {status.memoryMinFreeHeap != null && <div className="aq-info-row"><span>{T.minFree}</span><span>{formatBytes(status.memoryMinFreeHeap)}</span></div>}
                {status.cacheSize != null && <div className="aq-info-row"><span>{T.cacheSizeLabel}</span><span>{formatBytes(status.cacheSize)}</span></div>}
                {status.cacheAge != null && <div className="aq-info-row"><span>{T.cacheAgeLabel}</span><span>{status.cacheAge} ms</span></div>}
                {status.cacheValid != null && <div className="aq-info-row"><span>{T.cacheStatusLabel}</span><span>{status.cacheValid ? T.cacheActive : T.cacheInactive}</span></div>}
              </div>
            )}
          </>
        )}

        {/* ── SETTINGS TAB ── */}
        {tab === 'settings' && status && (
          <>
            {/* 1. Language */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="9"/>
                    <path d="M3 12h18"/>
                    <path d="M12 3c-2.5 3-4 5.7-4 9s1.5 6 4 9"/>
                    <path d="M12 3c2.5 3 4 5.7 4 9s-1.5 6-4 9"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.langTitle}</div>
                  <div className="aq-section-sub">{T.langSub}</div>
                </div>
              </div>
              <div className="aq-lang-stack">
                <button
                  className={`aq-lang-stack-btn ${lang === 'uk' ? 'active' : ''}`}
                  onClick={() => { setLang('uk'); localStorage.setItem('aq_lang', 'uk') }}
                >UK</button>
                <button
                  className={`aq-lang-stack-btn ${lang === 'en' ? 'active' : ''}`}
                  onClick={() => { setLang('en'); localStorage.setItem('aq_lang', 'en') }}
                >EN</button>
              </div>
            </div>

            {/* 2. WiFi */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <rect x="3" y="11" width="18" height="10" rx="2"/>
                    <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.wifiSettings}</div>
                  <div className="aq-section-sub">{T.wifiSettingsSub}</div>
                </div>
              </div>
              <div className="aq-info-row"><span>{T.ssid}</span><span>{status.wifiSSID || '—'}</span></div>
              <div className="aq-info-row"><span>{T.ipAddr}</span><span className="aq-mono">{status.wifiIP || '—'}</span></div>
              <button className="aq-save-btn" onClick={() => showToast(T.toastCommandSent)}>{T.forgetNetwork}</button>
              <p className="aq-settings-hint">{T.forgetHint}</p>
            </div>

            {/* 3. Power + Display */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M18.36 6.64A9 9 0 1 1 5.64 19.36"/>
                    <path d="M12 2v4"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.powerSettings}</div>
                  <div className="aq-section-sub">{T.powerSettingsSub}</div>
                </div>
              </div>
              <label className="aq-checkbox-row">
                <span>{T.powerSaveToggle}</span>
                <input
                  type="checkbox" className="aq-checkbox"
                  checked={status.powerSaveMode}
                  onChange={async () => {
                    try {
                      await api.setDisplaySettings({
                        powerSaveMode: !status.powerSaveMode,
                        deepSleepIdleSec: sDeepSleep,
                        displayEnabled: sDisplayEnabled,
                        displayOffAfterSec: sDisplayOff,
                      })
                      showToast(T.toastPowerSave(!status.powerSaveMode))
                      fetchStatus()
                    } catch { showToast(T.toastError) }
                  }}
                />
              </label>
              <p className="aq-settings-hint">{T.powerSaveHint}</p>
              <div className="aq-settings-field">
                <label className="aq-settings-label">{T.deepSleepLabel}</label>
                <input
                  type="number" className="aq-settings-input"
                  min="10" max="3600" value={sDeepSleep}
                  onChange={e => setSDeepSleep(Number(e.target.value))}
                />
              </div>
              <label className="aq-checkbox-row" style={{ marginTop: 10 }}>
                <span>{T.oledToggle}</span>
                <input
                  type="checkbox" className="aq-checkbox"
                  checked={sDisplayEnabled}
                  onChange={e => setSDisplayEnabled(e.target.checked)}
                />
              </label>
              <p className="aq-settings-hint">{T.oledHint}</p>
              <div className="aq-settings-field">
                <label className="aq-settings-label">{T.displayOffLabel(status.displayOffAfterSec)}</label>
                <input
                  type="number" className="aq-settings-input"
                  min="5" max="600" value={sDisplayOff}
                  onChange={e => setSDisplayOff(Number(e.target.value))}
                />
              </div>
              <button className="aq-save-btn" style={{ marginTop: 12 }} onClick={async () => {
                try {
                  await api.setDisplaySettings({
                    powerSaveMode: status.powerSaveMode,
                    deepSleepIdleSec: sDeepSleep,
                    displayEnabled: sDisplayEnabled,
                    displayOffAfterSec: sDisplayOff,
                  })
                  showToast(T.toastSaved)
                  fetchStatus()
                } catch { showToast(T.toastSaveError) }
              }}>{T.savePowerSettings}</button>
            </div>

            {/* 4. Min interval */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="7"/>
                    <line x1="12" y1="12" x2="12" y2="7.5"/>
                    <line x1="12" y1="12" x2="15.5" y2="13.5"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.feedInterval}</div>
                  <div className="aq-section-sub">{T.feedIntervalSub}</div>
                </div>
              </div>
              <div className="aq-settings-field">
                <label className="aq-settings-label">{T.minIntervalLabel}</label>
                <input
                  type="number" className="aq-settings-input"
                  min="1" max="1440" value={sMinInterval}
                  onChange={e => setSMinInterval(Number(e.target.value))}
                />
              </div>
              <p className="aq-settings-hint">{T.minIntervalHint(status.minFeedIntervalMin)}</p>
              <button className="aq-save-btn" onClick={async () => {
                try {
                  await api.setMinInterval({ minFeedIntervalMin: sMinInterval })
                  showToast(T.toastIntervalSaved)
                  fetchStatus()
                } catch { showToast(T.toastSaveError) }
              }}>{T.saveInterval}</button>
            </div>

            {/* 5. Battery calibration */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <rect x="5" y="8" width="12" height="8" rx="2"/>
                    <path d="M17 11h2.2a1 1 0 0 1 0 2H17"/>
                    <rect x="7.5" y="10" width="5" height="4" rx="1" fill="#4A5568" stroke="none"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.calibration}</div>
                  <div className="aq-section-sub">{T.calibrationSub}</div>
                </div>
              </div>
              <p className="aq-settings-hint" style={{ marginTop: 0, marginBottom: 8 }}><strong>{T.calibrationCurrent(status.batteryVoltage.toFixed(2))}</strong></p>
              <div className="aq-settings-field">
                <label className="aq-settings-label">{T.calibrationLabel}</label>
                <input
                  type="number" className="aq-settings-input"
                  step="0.01" min="2.5" max="4.5"
                  value={sCalibVoltage || status.batteryVoltage.toFixed(2)}
                  onChange={e => setSCalibVoltage(e.target.value)}
                />
              </div>
              <p className="aq-settings-hint">{T.calibrationHint}</p>
              <button className="aq-save-btn" onClick={async () => {
                const v = parseFloat(sCalibVoltage || status.batteryVoltage.toFixed(2))
                if (isNaN(v) || v < 2.5 || v > 4.5) { showToast(T.calibrationInvalid); return }
                try {
                  await api.calibrateBattery({ actualVoltage: v })
                  setSCalibVoltage('')
                  showToast(T.toastCalibDone)
                  fetchStatus()
                } catch { showToast(T.toastCalibError) }
              }}>{T.applyCalibration}</button>
            </div>

            {/* 6. Timezone */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="9"/>
                    <line x1="12" y1="3" x2="12" y2="7"/>
                    <line x1="12" y1="17" x2="12" y2="21"/>
                    <line x1="3" y1="12" x2="7" y2="12"/>
                    <line x1="17" y1="12" x2="21" y2="12"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.timezone}</div>
                  <div className="aq-section-sub">{T.timezoneSub}</div>
                </div>
              </div>
              <div className="aq-settings-field">
                <label className="aq-settings-label">{T.timezoneLabel}</label>
                <select className="aq-settings-input" value={sTimezone} onChange={e => setSTimezone(Number(e.target.value))}>
                  {Array.from({ length: 27 }, (_, i) => i - 12).map(offset => (
                    <option key={offset} value={offset}>
                      UTC{offset >= 0 ? '+' : ''}{String(Math.abs(offset)).padStart(2, '0')}:00
                    </option>
                  ))}
                </select>
              </div>
              <p className="aq-settings-hint">{T.timezoneHint}</p>
              <button className="aq-save-btn" onClick={async () => {
                try {
                  await api.setTimezone({ offsetHours: sTimezone })
                  showToast(T.toastTimezoneSaved)
                } catch { showToast(T.toastSaveError) }
              }}>{T.saveTimezone}</button>
            </div>

            {/* Sign out */}
            <div className="aq-card">
              <div className="aq-section-header">
                <div className="aq-section-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/>
                    <polyline points="16 17 21 12 16 7"/>
                    <line x1="21" y1="12" x2="9" y2="12"/>
                  </svg>
                </div>
                <div>
                  <div className="aq-section-title">{T.account}</div>
                  <div className="aq-section-sub">{T.accountSub}</div>
                </div>
              </div>
              <button className="aq-signout-btn" onClick={handleLogout}>{T.signOut}</button>
            </div>
          </>
        )}
      </div>

      <nav className="aq-bottom-tabs">
        <button className={`aq-tab ${tab === 'home' ? 'active' : ''}`} onClick={() => setTab('home')}>
          <svg className="aq-tab-icon" viewBox="0 0 24 24">
            <path d="M10 20V14H14V20H19V12H22L12 3L2 12H5V20H10Z" fill="currentColor"/>
          </svg>
          <span>{T.tabHome}</span>
        </button>
        <button className={`aq-tab ${tab === 'info' ? 'active' : ''}`} onClick={() => setTab('info')}>
          <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.9" strokeLinecap="round" strokeLinejoin="round">
            <circle cx="12" cy="12" r="10"/>
            <path d="M12 16v-4"/>
            <circle cx="12" cy="8" r="1" fill="currentColor" stroke="none"/>
          </svg>
          <span>{T.tabInfo}</span>
        </button>
        <button className={`aq-tab ${tab === 'settings' ? 'active' : ''}`} onClick={() => setTab('settings')}>
          <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.9" strokeLinecap="round" strokeLinejoin="round">
            <path d="M2.5 9.2C7.03 4.66 16.97 4.66 21.5 9.2"/>
            <path d="M5.8 12.5C9.12 9.19 14.88 9.19 18.2 12.5"/>
            <path d="M9.4 15.9C11.15 14.15 12.85 14.15 14.6 15.9"/>
            <circle cx="12" cy="19.2" r="1.2" fill="currentColor" stroke="none"/>
          </svg>
          <span>{T.tabSettings}</span>
        </button>
      </nav>
    </div>
  )
}

export default App
