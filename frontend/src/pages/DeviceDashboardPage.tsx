import { useCallback, useEffect, useRef, useState } from 'react'
import { Link, useParams } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'
import { useAuthStore } from '../store/authStore'
import { TRANSLATIONS, type Lang } from '../translations'
import type { FeedTime, StatusResponse, Device } from '../types'

// ─── Types ────────────────────────────────────────────────────────────────────

type Tab = 'home' | 'info' | 'settings'

interface LocalFeedTime extends FeedTime {
  day: string
}

// ─── Battery gauge SVG ────────────────────────────────────────────────────────

const GAUGE_R = 55
const GAUGE_CX = 60
const GAUGE_CY = 60
const GAUGE_START = 210  // degrees
const GAUGE_ARC = 300    // total sweep degrees
const CIRCUMFERENCE = 2 * Math.PI * GAUGE_R

function describeArc(cx: number, cy: number, r: number, startDeg: number, endDeg: number): string {
  const toRad = (d: number) => (d * Math.PI) / 180
  const x1 = cx + r * Math.cos(toRad(startDeg))
  const y1 = cy + r * Math.sin(toRad(startDeg))
  const x2 = cx + r * Math.cos(toRad(endDeg))
  const y2 = cy + r * Math.sin(toRad(endDeg))
  const large = endDeg - startDeg > 180 ? 1 : 0
  return `M ${x1} ${y1} A ${r} ${r} 0 ${large} 1 ${x2} ${y2}`
}

const TRACK_PATH = describeArc(GAUGE_CX, GAUGE_CY, GAUGE_R, GAUGE_START, GAUGE_START + GAUGE_ARC)

interface BatteryGaugeProps {
  percent: number
  isCharging: boolean
  voltage: number
  T: ReturnType<typeof useTranslations>
}

function useTranslations() {
  const [lang] = useState<Lang>(() => (localStorage.getItem('aq_lang') as Lang) || 'uk')
  return TRANSLATIONS[lang]
}

function BatteryGauge({ percent, isCharging, voltage, T }: BatteryGaugeProps) {
  const clamp = Math.max(0, Math.min(100, percent))
  const fillFraction = clamp / 100
  const trackLen = (GAUGE_ARC / 360) * CIRCUMFERENCE
  const offset = trackLen * (1 - fillFraction)

  const color = clamp >= 50 ? '#4CAF50' : clamp >= 20 ? '#FF9800' : '#f44336'

  return (
    <div className="aq-gauge-wrap">
      <svg className="aq-gauge-svg" viewBox="0 0 120 100">
        <path d={TRACK_PATH} fill="none" stroke="#e8eaed" strokeWidth="9" strokeLinecap="round" />
        <path
          d={TRACK_PATH}
          fill="none"
          stroke={color}
          strokeWidth="9"
          strokeLinecap="round"
          strokeDasharray={`${trackLen}`}
          strokeDashoffset={offset}
          className={isCharging ? 'aq-gauge-charging-arc' : 'aq-gauge-fill-arc'}
          style={isCharging ? {
            '--charge-full': `${trackLen}`,
            '--charge-target': `${offset}`,
          } as React.CSSProperties : undefined}
        />
        <text x="60" y="58" textAnchor="middle" fontSize="18" fontWeight="700" fill="#111">{clamp}%</text>
        <text x="60" y="73" textAnchor="middle" fontSize="10" fill="#888">{voltage.toFixed(2)}V</text>
      </svg>
      <div className="aq-gauge-title">{T.batteryStatus}</div>
    </div>
  )
}

interface NextFeedGaugeProps {
  nextFeedMinutes: number | null | undefined
  T: ReturnType<typeof useTranslations>
}

function NextFeedGauge({ nextFeedMinutes, T }: NextFeedGaugeProps) {
  const MAX_MINUTES = 24 * 60
  const minutes = nextFeedMinutes ?? null
  const fraction = minutes != null ? Math.max(0, Math.min(1, 1 - minutes / MAX_MINUTES)) : 0
  const trackLen = (GAUGE_ARC / 360) * CIRCUMFERENCE
  const offset = trackLen * (1 - fraction)

  const h = minutes != null ? Math.floor(minutes / 60) : 0
  const m = minutes != null ? minutes % 60 : 0
  const label = minutes != null ? T.nextFeedIn(h, m) : '--'

  return (
    <div className="aq-gauge-wrap">
      <svg className="aq-gauge-svg" viewBox="0 0 120 100">
        <path d={TRACK_PATH} fill="none" stroke="#e8eaed" strokeWidth="9" strokeLinecap="round" />
        <path
          d={TRACK_PATH}
          fill="none"
          stroke="#1976D2"
          strokeWidth="9"
          strokeLinecap="round"
          strokeDasharray={`${trackLen}`}
          strokeDashoffset={offset}
          className="aq-gauge-fill-arc"
        />
        <text x="60" y="55" textAnchor="middle" fontSize="11" fontWeight="700" fill="#111">{label}</text>
        <text x="60" y="70" textAnchor="middle" fontSize="9" fill="#888">{T.untilNextFeed}</text>
      </svg>
      <div className="aq-gauge-title">{T.schedule}</div>
    </div>
  )
}

// ─── Food helpers ─────────────────────────────────────────────────────────────

function calcFoodRemaining(totalG: number, loadedTs: number, gpf: number, feedTimesPerDay: number): number {
  if (!totalG || !loadedTs || !gpf || !feedTimesPerDay) return totalG
  const elapsed = (Date.now() - loadedTs) / 1000 / 3600 / 24 // days
  const consumed = elapsed * feedTimesPerDay * gpf
  return Math.max(0, totalG - consumed)
}

function calcFoodDuration(remainingG: number, gpf: number, feedTimesPerDay: number): { months: number; days: number } {
  if (!gpf || !feedTimesPerDay || !remainingG) return { months: 0, days: 0 }
  const totalDays = remainingG / (gpf * feedTimesPerDay)
  const months = Math.floor(totalDays / 30)
  const days = Math.round(totalDays % 30)
  return { months, days }
}

// ─── Tab icons ────────────────────────────────────────────────────────────────

const HomeIcon = () => (
  <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z" />
    <polyline points="9 22 9 12 15 12 15 22" />
  </svg>
)

const InfoIcon = () => (
  <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <circle cx="12" cy="12" r="10" />
    <line x1="12" y1="16" x2="12" y2="12" />
    <line x1="12" y1="8" x2="12.01" y2="8" />
  </svg>
)

const SettingsIcon = () => (
  <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <circle cx="12" cy="12" r="3" />
    <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
  </svg>
)

// ─── Main component ───────────────────────────────────────────────────────────

export function DeviceDashboardPage() {
  const { deviceId } = useParams<{ deviceId: string }>()
  const logout = useAuthStore((s) => s.logout)

  // ── Language ──────────────────────────────────────────────────────────────
  const [lang, setLang] = useState<Lang>(() => (localStorage.getItem('aq_lang') as Lang) || 'uk')
  const T = TRANSLATIONS[lang]

  const switchLang = (l: Lang) => {
    setLang(l)
    localStorage.setItem('aq_lang', l)
  }

  // ── Core state ────────────────────────────────────────────────────────────
  const [device, setDevice] = useState<Device | null>(null)
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  // ── Tab ───────────────────────────────────────────────────────────────────
  const [tab, setTab] = useState<Tab>('home')

  // ── Feed ──────────────────────────────────────────────────────────────────
  const [feeding, setFeeding] = useState(false)
  const [feedRepeats, setFeedRepeats] = useState(1)
  const [cooldown, setCooldown] = useState(0)

  // ── Schedule ──────────────────────────────────────────────────────────────
  const [localSchedule, setLocalSchedule] = useState<LocalFeedTime[]>([])
  const scheduleEdited = useRef(false)

  // ── Toast ─────────────────────────────────────────────────────────────────
  const [toastMsg, setToastMsg] = useState('')
  const [toastVisible, setToastVisible] = useState(false)
  const toastTimer = useRef<ReturnType<typeof setTimeout> | null>(null)

  // ── Settings ──────────────────────────────────────────────────────────────
  const [sDeepSleep, setSDeepSleep] = useState(300)
  const [sDisplayEnabled, setSDisplayEnabled] = useState(true)
  const [sDisplayOff, setSDisplayOff] = useState(30)
  const [sPowerSave, setSPowerSave] = useState(false)
  const [sMinInterval, setSMinInterval] = useState(5)
  const [sCalibVoltage, setSCalibVoltage] = useState('')
  const [sTimezone, setSTimezone] = useState(2)
  const settingsInited = useRef(false)

  // ── Servo (tracked from status but not surfaced in UI on this page) ────────
  const [_angle, setAngle] = useState(90)
  const [_speed, setSpeed] = useState(50)

  // ── Food ──────────────────────────────────────────────────────────────────
  const [foodTotalG, setFoodTotalG] = useState<number>(() => Number(localStorage.getItem('aq_food_total') || 0))
  const [foodLoadedTs, setFoodLoadedTs] = useState<number>(() => Number(localStorage.getItem('aq_food_ts') || 0))
  const [gramsPerFeed, setGramsPerFeed] = useState<number>(() => Number(localStorage.getItem('aq_food_gpf') || 0))
  const [foodFormOpen, setFoodFormOpen] = useState(false)
  const [foodInputG, setFoodInputG] = useState('')
  const [foodInputGpf, setFoodInputGpf] = useState('')

  // ─── Toast helper ──────────────────────────────────────────────────────────
  const showToast = useCallback((msg: string) => {
    setToastMsg(msg)
    setToastVisible(true)
    if (toastTimer.current) clearTimeout(toastTimer.current)
    toastTimer.current = setTimeout(() => setToastVisible(false), 2200)
  }, [])

  // ─── Fetch device ──────────────────────────────────────────────────────────
  useEffect(() => {
    if (!deviceId) return
    void api.getDevice(deviceId).then(setDevice).catch((err) => {
      setError(getApiErrorMessage(err, 'Device not found'))
    })
  }, [deviceId])

  // ─── Fetch status ──────────────────────────────────────────────────────────
  const fetchStatus = useCallback(async () => {
    try {
      const data = await api.getStatus()
      setStatus(data)

      if (!scheduleEdited.current) {
        setLocalSchedule(
          data.feedTimes.map((ft) => ({ ...ft, day: '0' }))
        )
      }

      setAngle(data.angle)
      setSpeed(data.speed)

      if (!settingsInited.current) {
        setSDeepSleep(data.deepSleepIdleSec)
        setSDisplayEnabled(data.displayEnabled)
        setSDisplayOff(data.displayOffAfterSec)
        setSPowerSave(data.powerSaveMode)
        setSMinInterval(data.minFeedIntervalMin)
        settingsInited.current = true
      }

      setCooldown(data.manualFeedCooldownSeconds > 0 ? data.manualFeedCooldownSeconds : 0)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Failed to load status'))
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    void fetchStatus()
    const interval = setInterval(() => void fetchStatus(), 5000)
    return () => clearInterval(interval)
  }, [fetchStatus])

  // ─── Cooldown countdown ────────────────────────────────────────────────────
  useEffect(() => {
    if (cooldown <= 0) return
    const t = setInterval(() => {
      setCooldown((c) => {
        if (c <= 1) { clearInterval(t); return 0 }
        return c - 1
      })
    }, 1000)
    return () => clearInterval(t)
  }, [cooldown])

  // ─── Feed now ─────────────────────────────────────────────────────────────
  const handleFeed = async () => {
    if (feeding || cooldown > 0) return
    try {
      setFeeding(true)
      showToast(T.toastFeeding)
      await api.feedNow({ repeats: feedRepeats })
      showToast(T.toastFed)
      setTimeout(() => {
        setFeeding(false)
        void fetchStatus()
      }, 2000)
    } catch (err) {
      showToast(T.toastFeedError)
      setFeeding(false)
    }
  }

  // ─── Schedule helpers ──────────────────────────────────────────────────────
  const addFeedTime = () => {
    scheduleEdited.current = true
    setLocalSchedule((prev) => [...prev, { hour: 8, minute: 0, repeats: 1, day: '0' }])
    showToast(T.toastFeedAdded)
  }

  const removeFeedTime = (idx: number) => {
    scheduleEdited.current = true
    setLocalSchedule((prev) => prev.filter((_, i) => i !== idx))
    showToast(T.toastFeedRemoved)
  }

  const updateFeedTime = (idx: number, field: keyof LocalFeedTime, value: string | number) => {
    scheduleEdited.current = true
    setLocalSchedule((prev) =>
      prev.map((ft, i) => i === idx ? { ...ft, [field]: value } : ft)
    )
  }

  const saveSchedule = async () => {
    try {
      await api.setSchedule({
        times: localSchedule.map((ft) => ({ hour: ft.hour, minute: ft.minute, repeats: ft.repeats })),
      })
      showToast(T.toastScheduleSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  // ─── Settings save ─────────────────────────────────────────────────────────
  const savePowerSettings = async () => {
    try {
      await api.setDisplaySettings({
        powerSaveMode: sPowerSave,
        deepSleepIdleSec: sDeepSleep,
        displayEnabled: sDisplayEnabled,
        displayOffAfterSec: sDisplayOff,
      })
      showToast(T.toastSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const saveMinInterval = async () => {
    try {
      await api.setMinInterval({ minFeedIntervalMin: sMinInterval })
      showToast(T.toastIntervalSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const applyCalibration = async () => {
    const v = parseFloat(sCalibVoltage)
    if (isNaN(v) || v < 2.5 || v > 4.5) {
      showToast(T.calibrationInvalid)
      return
    }
    try {
      await api.calibrateBattery({ actualVoltage: v })
      showToast(T.toastCalibDone)
    } catch {
      showToast(T.toastCalibError)
    }
  }

  const saveTimezone = async () => {
    try {
      await api.setTimezone({ offsetHours: sTimezone })
      showToast(T.toastTimezoneSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  // ─── Food save ─────────────────────────────────────────────────────────────
  const saveFoodForm = () => {
    const g = parseFloat(foodInputG)
    const gpf = parseFloat(foodInputGpf)
    if (!isNaN(g) && g > 0) {
      const ts = Date.now()
      setFoodTotalG(g)
      setFoodLoadedTs(ts)
      localStorage.setItem('aq_food_total', String(g))
      localStorage.setItem('aq_food_ts', String(ts))
    }
    if (!isNaN(gpf) && gpf > 0) {
      setGramsPerFeed(gpf)
      localStorage.setItem('aq_food_gpf', String(gpf))
    }
    setFoodFormOpen(false)
    setFoodInputG('')
    setFoodInputGpf('')
  }

  // ─── Food calculations ─────────────────────────────────────────────────────
  const feedTimesPerDay = localSchedule.filter((ft) => ft.day === '0').length || localSchedule.length
  const foodRemaining = foodTotalG > 0
    ? calcFoodRemaining(foodTotalG, foodLoadedTs, gramsPerFeed, feedTimesPerDay || 1)
    : 0
  const foodPercent = foodTotalG > 0 ? Math.min(100, (foodRemaining / foodTotalG) * 100) : 0
  const foodBarColor = foodPercent > 50 ? '#4CAF50' : foodPercent > 20 ? '#FF9800' : '#f44336'
  const { months: foodMonths, days: foodDays } = foodTotalG > 0 && gramsPerFeed > 0
    ? calcFoodDuration(foodRemaining, gramsPerFeed, feedTimesPerDay || 1)
    : { months: 0, days: 0 }

  // ─── Uptime ────────────────────────────────────────────────────────────────
  const uptimeH = status ? Math.floor(status.deepSleepIdleSec / 3600) : 0
  const uptimeM = status ? Math.floor((status.deepSleepIdleSec % 3600) / 60) : 0

  // ─── Guard ────────────────────────────────────────────────────────────────
  if (!deviceId) {
    return <div className="error-banner">No device ID</div>
  }

  if (loading && !status) {
    return (
      <div className="aq-loading">
        <div className="aq-spinner" />
        <div className="aq-loading-text">{T.connecting}</div>
      </div>
    )
  }

  // ═══════════════════════════════════════════════════════════════════════════
  //  HOME TAB
  // ═══════════════════════════════════════════════════════════════════════════

  const renderHome = () => (
    <>
      {/* Battery card */}
      <div className="aq-card aq-battery-card">
        <div className="aq-gauges-row">
          <BatteryGauge
            percent={status?.batteryPercent ?? 0}
            isCharging={status?.isCharging ?? false}
            voltage={status?.batteryVoltage ?? 0}
            T={T}
          />
          <NextFeedGauge
            nextFeedMinutes={status?.nextFeedMinutes}
            T={T}
          />
        </div>

        {/* Food supply */}
        <div className="aq-food-section">
          <div className="aq-food-header">
            <span className="aq-food-title">{T.foodTitle}</span>
            <button
              className="aq-food-refill-btn"
              onClick={() => {
                setFoodFormOpen((o) => !o)
                setFoodInputG(foodTotalG > 0 ? String(foodTotalG) : '')
                setFoodInputGpf(gramsPerFeed > 0 ? String(gramsPerFeed) : '')
              }}
            >
              {T.foodRefill}
            </button>
          </div>

          {foodTotalG > 0 ? (
            <>
              <div className="aq-food-bar-track">
                <div
                  className="aq-food-bar-fill"
                  style={{ width: `${foodPercent}%`, background: foodBarColor }}
                />
              </div>
              <div className="aq-food-meta">
                <span>{T.foodRemaining(Math.round(foodRemaining))}</span>
                <span className="aq-food-meta-right">
                  {gramsPerFeed > 0 && feedTimesPerDay > 0
                    ? T.foodDuration(foodMonths, foodDays)
                    : T.foodNoSchedule}
                </span>
              </div>
              {feedTimesPerDay > 0 && (
                <div className="aq-food-hint">{T.foodFeedingsPerDay(feedTimesPerDay)}</div>
              )}
            </>
          ) : (
            <div className="aq-food-hint">{T.foodNotSet}</div>
          )}

          {foodFormOpen && (
            <div className="aq-food-form">
              <div className="aq-food-form-row">
                <label className="aq-food-form-label">{T.foodGramsTotal}</label>
                <input
                  className="aq-food-form-input"
                  type="number"
                  min="1"
                  value={foodInputG}
                  onChange={(e) => setFoodInputG(e.target.value)}
                  placeholder="г"
                />
              </div>
              <div className="aq-food-form-row">
                <label className="aq-food-form-label">{T.foodGramsPerFeed}</label>
                <input
                  className="aq-food-form-input"
                  type="number"
                  min="0.1"
                  step="0.1"
                  value={foodInputGpf}
                  onChange={(e) => setFoodInputGpf(e.target.value)}
                  placeholder="г"
                />
              </div>
              <div className="aq-food-form-btns">
                <button className="aq-food-cancel-btn" onClick={() => setFoodFormOpen(false)}>
                  {T.foodCancel}
                </button>
                <button className="aq-food-save-btn" onClick={saveFoodForm}>
                  {T.foodSave}
                </button>
              </div>
            </div>
          )}
        </div>

        {status?.isCharging && (
          <div className="aq-alert aq-alert-charging">
            <span>⚡</span>
            {T.isCharging}
          </div>
        )}
        {!status?.isCharging && (status?.batteryPercent ?? 100) < 20 && (
          <div className="aq-alert aq-alert-low">
            <span>!</span>
            {T.lowBattery}
          </div>
        )}
      </div>

      {/* Manual feed card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M12 2a10 10 0 1 0 0 20A10 10 0 0 0 12 2z" />
              <path d="M12 8v4l3 3" />
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
            <button
              className="aq-stepper-btn"
              onClick={() => setFeedRepeats((r) => Math.max(1, r - 1))}
              disabled={feedRepeats <= 1}
            >−</button>
            <span className="aq-stepper-val">{feedRepeats}</span>
            <button
              className="aq-stepper-btn"
              onClick={() => setFeedRepeats((r) => Math.min(10, r + 1))}
              disabled={feedRepeats >= 10}
            >+</button>
          </div>
        </div>

        <button
          className={`aq-feed-btn${feeding ? ' is-feeding' : ''}`}
          onClick={() => void handleFeed()}
          disabled={feeding || cooldown > 0}
        >
          {feeding
            ? T.feeding
            : cooldown > 0
              ? T.feedNowCooldown(cooldown)
              : T.feedNow}
        </button>
      </div>

      {/* Schedule card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <rect x="3" y="4" width="18" height="18" rx="2" ry="2" />
              <line x1="16" y1="2" x2="16" y2="6" />
              <line x1="8" y1="2" x2="8" y2="6" />
              <line x1="3" y1="10" x2="21" y2="10" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.schedule}</div>
            <div className="aq-section-sub">{T.scheduleSub}</div>
          </div>
        </div>

        {status?.nextFeedMinutes != null && (
          <div className="aq-info-banner" style={{ marginBottom: 10 }}>
            {T.nextFeedLabel(
              Math.floor(status.nextFeedMinutes / 60),
              status.nextFeedMinutes % 60,
            )}
          </div>
        )}

        <div className="aq-schedule-list">
          {localSchedule.map((ft, idx) => (
            <div className="aq-feed-block" key={idx}>
              <div className="aq-feed-time-col">
                <input
                  className="aq-time-input"
                  type="time"
                  value={`${String(ft.hour).padStart(2, '0')}:${String(ft.minute).padStart(2, '0')}`}
                  onChange={(e) => {
                    const [h, m] = e.target.value.split(':').map(Number)
                    updateFeedTime(idx, 'hour', h)
                    updateFeedTime(idx, 'minute', m)
                  }}
                />
              </div>
              <div className="aq-feed-center">
                <div className="aq-feed-field">
                  <span>{T.repeats}</span>
                  <div className="aq-stepper aq-stepper-sm">
                    <button
                      className="aq-stepper-btn aq-stepper-btn-sm"
                      onClick={() => updateFeedTime(idx, 'repeats', Math.max(1, ft.repeats - 1))}
                      disabled={ft.repeats <= 1}
                    >−</button>
                    <span className="aq-stepper-val">{ft.repeats}</span>
                    <button
                      className="aq-stepper-btn aq-stepper-btn-sm"
                      onClick={() => updateFeedTime(idx, 'repeats', Math.min(10, ft.repeats + 1))}
                      disabled={ft.repeats >= 10}
                    >+</button>
                  </div>
                </div>
                <div className="aq-feed-field">
                  <span>{T.day}</span>
                  <select
                    className="aq-day-select"
                    value={ft.day}
                    onChange={(e) => updateFeedTime(idx, 'day', e.target.value)}
                  >
                    {T.days.map((d, i) => (
                      <option key={i} value={String(i)}>{d}</option>
                    ))}
                  </select>
                </div>
              </div>
              <button className="aq-remove-btn" onClick={() => removeFeedTime(idx)}>×</button>
            </div>
          ))}
        </div>

        <button className="aq-add-btn" onClick={addFeedTime}>{T.addFeeding}</button>
        <button className="aq-save-btn" onClick={() => void saveSchedule()}>{T.saveAllTimes}</button>
      </div>
    </>
  )

  // ═══════════════════════════════════════════════════════════════════════════
  //  INFO TAB
  // ═══════════════════════════════════════════════════════════════════════════

  const renderInfo = () => (
    <>
      {/* WiFi card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M5 12.55a11 11 0 0 1 14.08 0" />
              <path d="M1.42 9a16 16 0 0 1 21.16 0" />
              <path d="M8.53 16.11a6 6 0 0 1 6.95 0" />
              <line x1="12" y1="20" x2="12.01" y2="20" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.wifiInfo}</div>
            <div className="aq-section-sub">{T.wifiInfoSub}</div>
          </div>
        </div>
        <div className="aq-info-row">
          <span>{T.ssid}</span>
          <span>{status?.wifiSSID || T.notConfigured}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.ipAddr}</span>
          <span className="aq-mono">{status?.wifiIP || T.notConnected}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.modeLabel}</span>
          <span>{status?.isAPMode ? T.apMode : T.staMode}</span>
        </div>
      </div>

      {/* Battery/device info card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <rect x="2" y="7" width="16" height="10" rx="2" ry="2" />
              <line x1="22" y1="11" x2="22" y2="13" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.batterySection}</div>
            <div className="aq-section-sub">{T.batterySectionSub}</div>
          </div>
        </div>
        <div className="aq-info-row">
          <span>{T.voltageValue}</span>
          <span>{status?.batteryVoltage.toFixed(2)} V</span>
        </div>
        <div className="aq-info-row">
          <span>{T.percentValue}</span>
          <span>{status?.batteryPercent}%</span>
        </div>
      </div>

      {/* System info card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <rect x="2" y="3" width="20" height="14" rx="2" ry="2" />
              <line x1="8" y1="21" x2="16" y2="21" />
              <line x1="12" y1="17" x2="12" y2="21" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.systemInfo}</div>
            <div className="aq-section-sub">{T.systemInfoSub}</div>
          </div>
        </div>
        <div className="aq-info-row">
          <span>{T.uptime}</span>
          <span>{T.uptimeFmt(uptimeH, uptimeM)}</span>
        </div>
        {status?.currentTime && (
          <div className="aq-info-row">
            <span>{T.deviceTime}</span>
            <span className="aq-mono">{status.currentTime}</span>
          </div>
        )}
        <div className="aq-info-row">
          <span>{T.powerSaveLabel}</span>
          <span>{status?.powerSaveMode ? T.on : T.off}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.oledLabel}</span>
          <span>{status?.displayEnabled ? T.on : T.off}</span>
        </div>
      </div>
    </>
  )

  // ═══════════════════════════════════════════════════════════════════════════
  //  SETTINGS TAB
  // ═══════════════════════════════════════════════════════════════════════════

  const renderSettings = () => (
    <>
      {/* 1. Language */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="10" />
              <line x1="2" y1="12" x2="22" y2="12" />
              <path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.langTitle}</div>
            <div className="aq-section-sub">{T.langSub}</div>
          </div>
        </div>
        <div className="aq-lang-stack">
          <button
            className={`aq-lang-stack-btn${lang === 'uk' ? ' active' : ''}`}
            onClick={() => switchLang('uk')}
          >
            🇺🇦 Українська
          </button>
          <button
            className={`aq-lang-stack-btn${lang === 'en' ? ' active' : ''}`}
            onClick={() => switchLang('en')}
          >
            🇬🇧 English
          </button>
        </div>
      </div>

      {/* 2. WiFi */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M5 12.55a11 11 0 0 1 14.08 0" />
              <path d="M1.42 9a16 16 0 0 1 21.16 0" />
              <path d="M8.53 16.11a6 6 0 0 1 6.95 0" />
              <line x1="12" y1="20" x2="12.01" y2="20" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.wifiSettings}</div>
            <div className="aq-section-sub">{T.wifiSettingsSub}</div>
          </div>
        </div>
        <p className="aq-settings-hint">{T.forgetHint}</p>
      </div>

      {/* 3. Power settings */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M18.36 6.64a9 9 0 1 1-12.73 0" />
              <line x1="12" y1="2" x2="12" y2="12" />
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
            type="checkbox"
            className="aq-checkbox"
            checked={sPowerSave}
            onChange={(e) => setSPowerSave(e.target.checked)}
          />
        </label>
        <p className="aq-settings-hint">{T.powerSaveHint}</p>

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.deepSleepLabel}</label>
          <input
            className="aq-settings-input"
            type="number"
            min="10"
            max="3600"
            value={sDeepSleep}
            onChange={(e) => setSDeepSleep(Number(e.target.value))}
          />
        </div>

        <label className="aq-checkbox-row" style={{ marginTop: 12 }}>
          <span>{T.oledToggle}</span>
          <input
            type="checkbox"
            className="aq-checkbox"
            checked={sDisplayEnabled}
            onChange={(e) => setSDisplayEnabled(e.target.checked)}
          />
        </label>
        <p className="aq-settings-hint">{T.oledHint}</p>

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.displayOffLabel(sDisplayOff)}</label>
          <input
            className="aq-settings-input"
            type="number"
            min="5"
            max="600"
            value={sDisplayOff}
            onChange={(e) => setSDisplayOff(Number(e.target.value))}
          />
        </div>

        <button className="aq-save-btn" style={{ marginTop: 14 }} onClick={() => void savePowerSettings()}>
          {T.savePowerSettings}
        </button>
      </div>

      {/* 4. Feed interval */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="10" />
              <polyline points="12 6 12 12 16 14" />
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
            className="aq-settings-input"
            type="number"
            min="1"
            max="1440"
            value={sMinInterval}
            onChange={(e) => setSMinInterval(Number(e.target.value))}
          />
        </div>
        <p className="aq-settings-hint">{T.minIntervalHint(sMinInterval)}</p>
        <button className="aq-save-btn" onClick={() => void saveMinInterval()}>{T.saveInterval}</button>
      </div>

      {/* 5. Battery calibration */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <line x1="8" y1="6" x2="21" y2="6" />
              <line x1="8" y1="12" x2="21" y2="12" />
              <line x1="8" y1="18" x2="21" y2="18" />
              <line x1="3" y1="6" x2="3.01" y2="6" />
              <line x1="3" y1="12" x2="3.01" y2="12" />
              <line x1="3" y1="18" x2="3.01" y2="18" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.calibration}</div>
            <div className="aq-section-sub">{T.calibrationSub}</div>
          </div>
        </div>

        {status && (
          <p className="aq-settings-hint">{T.calibrationCurrent(status.batteryVoltage.toFixed(3))}</p>
        )}

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.calibrationLabel}</label>
          <input
            className="aq-settings-input"
            type="number"
            min="2.5"
            max="4.5"
            step="0.001"
            value={sCalibVoltage}
            onChange={(e) => setSCalibVoltage(e.target.value)}
            placeholder="e.g. 3.742"
          />
        </div>
        <p className="aq-settings-hint">{T.calibrationHint}</p>
        <button className="aq-save-btn" onClick={() => void applyCalibration()}>{T.applyCalibration}</button>
      </div>

      {/* 6. Timezone */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="10" />
              <line x1="2" y1="12" x2="22" y2="12" />
              <path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.timezone}</div>
            <div className="aq-section-sub">{T.timezoneSub}</div>
          </div>
        </div>

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.timezoneLabel}</label>
          <select
            className="aq-settings-input"
            value={sTimezone}
            onChange={(e) => setSTimezone(Number(e.target.value))}
          >
            {Array.from({ length: 27 }, (_, i) => i - 12).map((offset) => (
              <option key={offset} value={offset}>
                UTC{offset >= 0 ? `+${offset}` : offset}
              </option>
            ))}
          </select>
        </div>
        <p className="aq-settings-hint">{T.timezoneHint}</p>
        <button className="aq-save-btn" onClick={() => void saveTimezone()}>{T.saveTimezone}</button>
      </div>

      {/* 7. Account */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" />
              <circle cx="12" cy="7" r="4" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.account}</div>
            <div className="aq-section-sub">{T.accountSub}</div>
          </div>
        </div>
        <button
          className="aq-signout-btn"
          onClick={() => void logout()}
        >
          {T.signOut}
        </button>
      </div>
    </>
  )

  // ═══════════════════════════════════════════════════════════════════════════
  //  RENDER
  // ═══════════════════════════════════════════════════════════════════════════

  return (
    <div className="aq-shell">
      {/* Toast */}
      <div className={`aq-toast${toastVisible ? ' show' : ''}`}>
        <span className="aq-toast-icon">i</span>
        {toastMsg}
      </div>

      {/* Device mini-header */}
      <div style={{ padding: '10px 12px 0', maxWidth: 520, margin: '0 auto', width: '100%', boxSizing: 'border-box' }}>
        <Link to="/devices" className="back-link">← Мої акваріуми</Link>
        {device && (
          <div style={{ fontSize: 17, fontWeight: 700, color: '#1f2937', marginBottom: 4 }}>
            {device.name}
          </div>
        )}
        {error && <div className="error-banner">{error}</div>}
      </div>

      {/* Scrollable content */}
      <div className="aq-content">
        {tab === 'home' && renderHome()}
        {tab === 'info' && renderInfo()}
        {tab === 'settings' && renderSettings()}
      </div>

      {/* Bottom tabs */}
      <nav className="aq-bottom-tabs">
        <button
          className={`aq-tab${tab === 'home' ? ' active' : ''}`}
          onClick={() => setTab('home')}
        >
          <HomeIcon />
          {T.tabHome}
        </button>
        <button
          className={`aq-tab${tab === 'info' ? ' active' : ''}`}
          onClick={() => setTab('info')}
        >
          <InfoIcon />
          {T.tabInfo}
        </button>
        <button
          className={`aq-tab${tab === 'settings' ? ' active' : ''}`}
          onClick={() => setTab('settings')}
        >
          <SettingsIcon />
          {T.tabSettings}
        </button>
      </nav>
    </div>
  )
}
