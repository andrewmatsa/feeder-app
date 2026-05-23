import { useCallback, useEffect, useRef, useState } from 'react'
import { useNavigate, useParams, useSearchParams } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'
import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'
import { TRANSLATIONS, type Lang } from '../translations'
import type { StatusResponse, Device, DeviceFeedEvent, DeviceStats, LightStats } from '../types'

// ─── Types ────────────────────────────────────────────────────────────────────

type Tab = 'home' | 'info' | 'settings' | 'stats'

interface LocalFeedTime {
  hour: number
  minute: number
  repeats: number
  day: string  // UI select value: '0'=every day, '1'=Mon…'6'=Sat, '7'=Sun
}

// ─── Battery gauge SVG ────────────────────────────────────────────────────────

const GAUGE_R = 110
const GAUGE_CX = 130
const GAUGE_CY = 140
const GAUGE_START = 180
const GAUGE_ARC = 180
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
      <svg className="aq-gauge-svg" viewBox="0 0 260 160">
        <path d={TRACK_PATH} fill="none" stroke="#E6E9EF" strokeWidth="14" strokeLinecap="round" />
        <path
          d={TRACK_PATH}
          fill="none"
          stroke={color}
          strokeWidth="14"
          strokeLinecap="round"
          strokeDasharray={`${trackLen}`}
          strokeDashoffset={offset}
          className={isCharging ? 'aq-gauge-charging-arc' : 'aq-gauge-fill-arc'}
          style={isCharging ? {
            '--charge-full': `${trackLen}`,
            '--charge-target': `${offset}`,
          } as React.CSSProperties : undefined}
        />
        <text x="130" y="130" textAnchor="middle" dominantBaseline="middle" fontSize="28" fontWeight="700" fill={color}>{clamp}%</text>
      </svg>
      <div className="aq-gauge-title">{T.batteryStatus}</div>
      <div className="aq-gauge-sub">{T.voltageValue} {voltage.toFixed(2)} V</div>
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
      <svg className="aq-gauge-svg" viewBox="0 0 260 160">
        <path d={TRACK_PATH} fill="none" stroke="#E6E9EF" strokeWidth="14" strokeLinecap="round" />
        <path
          d={TRACK_PATH}
          fill="none"
          stroke="#1976D2"
          strokeWidth="14"
          strokeLinecap="round"
          strokeDasharray={`${trackLen}`}
          strokeDashoffset={offset}
          className="aq-gauge-fill-arc"
        />
        <text x="130" y="130" textAnchor="middle" dominantBaseline="middle" fontSize="28" fontWeight="600" fill="#1976D2">{label}</text>
      </svg>
      <div className="aq-gauge-title">{T.untilNextFeed}</div>
    </div>
  )
}

interface LightGaugeProps {
  durationSeconds: number
  isOn: boolean
  T: ReturnType<typeof useTranslations>
}

function LightGauge({ durationSeconds, isOn, T }: LightGaugeProps) {
  const MAX_SECONDS = 60 * 60
  const fraction = isOn ? Math.min(1, durationSeconds / MAX_SECONDS) : 0
  const trackLen = (GAUGE_ARC / 360) * CIRCUMFERENCE
  const offset = trackLen * (1 - fraction)
  const color = isOn ? '#D97706' : '#9CA3AF'
  const minutes = Math.floor(durationSeconds / 60)
  const label = isOn ? `${minutes}` : '--'

  return (
    <div className="aq-gauge-wrap">
      <svg className="aq-gauge-svg" viewBox="0 0 260 160">
        <path d={TRACK_PATH} fill="none" stroke="#E6E9EF" strokeWidth="14" strokeLinecap="round" />
        <path
          d={TRACK_PATH}
          fill="none"
          stroke={color}
          strokeWidth="14"
          strokeLinecap="round"
          strokeDasharray={`${trackLen}`}
          strokeDashoffset={offset}
          className="aq-gauge-fill-arc"
        />
        <text x="130" y="130" textAnchor="middle" dominantBaseline="middle" fontSize="28" fontWeight="700" fill={color}>
          {isOn ? `${minutes} хв` : '--'}
        </text>
      </svg>
      <div className="aq-gauge-title">{T.lightSensor}</div>
      <div className={`aq-gauge-sub aq-light-status${isOn ? ' aq-light-status-on' : ''}`}>
        {isOn ? T.lightOn : T.lightOff}
      </div>
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

const StatsIcon = () => (
  <svg className="aq-tab-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <line x1="18" y1="20" x2="18" y2="10" />
    <line x1="12" y1="20" x2="12" y2="4" />
    <line x1="6" y1="20" x2="6" y2="14" />
  </svg>
)

// ─── Sparkline ────────────────────────────────────────────────────────────────

function Sparkline({ data, id, color = '#667eea' }: { data: number[]; id: string; color?: string }) {
  if (data.length < 2) return null
  const W = 100
  const H = 36
  const max = Math.max(...data, 1)
  const pts: [number, number][] = data.map((v, i) => [
    (i / (data.length - 1)) * W,
    H - (v / max) * (H - 6) - 3,
  ])
  let linePath = `M ${pts[0][0]},${pts[0][1]}`
  for (let i = 1; i < pts.length; i++) {
    const dx = (pts[i][0] - pts[i - 1][0]) / 2.5
    linePath += ` C ${pts[i - 1][0] + dx},${pts[i - 1][1]} ${pts[i][0] - dx},${pts[i][1]} ${pts[i][0]},${pts[i][1]}`
  }
  const fillPath = `${linePath} L ${W},${H} L 0,${H} Z`
  const gradId = `sg-${id}`
  return (
    <svg viewBox={`0 0 ${W} ${H}`} preserveAspectRatio="none" style={{ width: '100%', height: 40, display: 'block', marginTop: 8 }} aria-hidden="true">
      <defs>
        <linearGradient id={gradId} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stopColor={color} stopOpacity="0.35" />
          <stop offset="100%" stopColor={color} stopOpacity="0" />
        </linearGradient>
      </defs>
      <path d={fillPath} fill={`url(#${gradId})`} />
      <path d={linePath} fill="none" stroke={color} strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  )
}

// ─── Main component ───────────────────────────────────────────────────────────

export function DeviceDashboardPage() {
  const { deviceId } = useParams<{ deviceId: string }>()
  const navigate = useNavigate()
  const logout = useAuthStore((s) => s.logout)
  const accountEmail = useAuthStore((s) => s.email)

  // ── Language ──────────────────────────────────────────────────────────────
  const [lang, setLang] = useState<Lang>(() => (localStorage.getItem('aq_lang') as Lang) || 'uk')
  const T = TRANSLATIONS[lang]

  const switchLang = (l: Lang) => {
    setLang(l)
    localStorage.setItem('aq_lang', l)
    setUiLang(l)
  }

  // ── Core state ────────────────────────────────────────────────────────────
  const [device, setDevice] = useState<Device | null>(null)
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [isOnline, setIsOnline] = useState<boolean | null>(null)
  const failCountRef = useRef(0)

  // ── Tab ───────────────────────────────────────────────────────────────────
  const setPageSubtitle = useUiStore(s => s.setPageSubtitle)
  const setUiLang = useUiStore(s => s.setLang)
  const [searchParams, setSearchParams] = useSearchParams()
  const tab = (searchParams.get('tab') as Tab | null) ?? 'home'
  const setTab = (t: Tab) => setSearchParams({ tab: t }, { replace: true })

  useEffect(() => {
    const labels: Record<Tab, string> = {
      home: T.tabHomeSubtitle,
      info: T.tabInfo,
      stats: T.tabStats,
      settings: T.tabSettings,
    }
    setPageSubtitle(labels[tab])
    return () => setPageSubtitle(T.tabHomeSubtitle)
  }, [tab, T, setPageSubtitle])

  // ── Feed ──────────────────────────────────────────────────────────────────
  const [feeding, setFeeding] = useState(false)
  const [feedRepeats, setFeedRepeats] = useState<number>(() =>
    Number(localStorage.getItem(`aq_feed_repeats_${deviceId}`) || 1)
  )
  const [cooldown, setCooldown] = useState(0)

  const updateFeedRepeats = useCallback((n: number) => {
    setFeedRepeats(n)
    localStorage.setItem(`aq_feed_repeats_${deviceId}`, String(n))
  }, [deviceId])

  // ── Food supply display toggle (localStorage) ─────────────────────────────
  const [sFoodEnabled, setSFoodEnabled] = useState(
    () => localStorage.getItem(`aq_food_enabled_${deviceId}`) !== 'false'
  )
  const handleFoodEnabled = (v: boolean) => {
    setSFoodEnabled(v)
    localStorage.setItem(`aq_food_enabled_${deviceId}`, String(v))
  }

  // ── Light sensor settings (localStorage) ─────────────────────────────────
  const [sLightEnabled, setSLightEnabled] = useState(
    () => localStorage.getItem(`aq_light_enabled_${deviceId}`) !== 'false'
  )
  const [sLightThreshold, setSLightThreshold] = useState(
    () => Number(localStorage.getItem(`aq_light_threshold_${deviceId}`) || 10)
  )
  const handleLightEnabled = (v: boolean) => {
    setSLightEnabled(v)
    localStorage.setItem(`aq_light_enabled_${deviceId}`, String(v))
  }
  const handleLightThreshold = (v: number) => {
    setSLightThreshold(v)
    localStorage.setItem(`aq_light_threshold_${deviceId}`, String(v))
  }

  // ── Light sensor duration tracking ────────────────────────────────────────
  const [lightOnSeconds, setLightOnSeconds] = useState(0)
  const lightSessionStartRef = useRef<number | null>(null)
  const lightSessionStartIsoRef = useRef<string | null>(null)

  // ── Schedule ──────────────────────────────────────────────────────────────
  const [localSchedule, setLocalSchedule] = useState<LocalFeedTime[]>([])
  const scheduleEdited = useRef(false)
  const [scheduleDirty, setScheduleDirty] = useState(false)

  // ── Feed journal ──────────────────────────────────────────────────────────
  const [feedEvents, setFeedEvents] = useState<DeviceFeedEvent[]>([])
  const [feedEventsLoading, setFeedEventsLoading] = useState(false)
  const [feedEventsError, setFeedEventsError] = useState<string | null>(null)

  // ── Statistics ────────────────────────────────────────────────────────────
  const [deviceStats, setDeviceStats] = useState<DeviceStats | null>(null)
  const [statsLoading, setStatsLoading] = useState(false)
  const [statsError, setStatsError] = useState<string | null>(null)
  const [lightStats, setLightStats] = useState<LightStats | null>(null)
  const [statsDays, setStatsDays] = useState(7)

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
  const [calibDirty, setCalibDirty] = useState(false)
  const [otaFile, setOtaFile] = useState<File | null>(null)
  const [otaState, setOtaState] = useState<'idle' | 'uploading' | 'rebooting' | 'done' | 'error'>('idle')
  const [otaError, setOtaError] = useState('')
  const [sTimezone, setSTimezone] = useState(2)
  const settingsInited = useRef(false)
  const [powerDirty, setPowerDirty] = useState(false)
  const [intervalDirty, setIntervalDirty] = useState(false)
  const [timezoneDirty, setTimezoneDirty] = useState(false)
  const [speedDirty, setSpeedDirty] = useState(false)

  // ── Servo ─────────────────────────────────────────────────────────────────
  const [sAngle, setSAngle] = useState(90)
  const [sSpeed, setSSpeed] = useState(20)
  const angleDebounce = useRef<ReturnType<typeof setTimeout> | null>(null)

  // ── Food ──────────────────────────────────────────────────────────────────
  const [foodTotalG, setFoodTotalG] = useState<number>(() => Number(localStorage.getItem('aq_food_total') || 0))
  const [foodLoadedTs, setFoodLoadedTs] = useState<number>(() => Number(localStorage.getItem('aq_food_ts') || 0))
  const [gramsPerFeed, setGramsPerFeed] = useState<number>(() => Number(localStorage.getItem('aq_food_gpf') || 0))

  const [foodInputG, setFoodInputG] = useState(() => localStorage.getItem('aq_food_total') || '')
  const [foodInputGpf, setFoodInputGpf] = useState(() => localStorage.getItem('aq_food_gpf') || '')
  const [forgetting, setForgetting] = useState(false)
  const [wifiForgetDone, setWifiForgetDone] = useState(false)

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
      const data = await api.getStatus(deviceId)
      failCountRef.current = 0
      setIsOnline(true)
      setStatus(data)

      const isLightOn = (data.lightLux ?? 0) > sLightThreshold
      if (isLightOn) {
        if (lightSessionStartRef.current === null) {
          lightSessionStartRef.current = Date.now()
          lightSessionStartIsoRef.current = new Date().toISOString()
        }
        setLightOnSeconds(Math.floor((Date.now() - lightSessionStartRef.current) / 1000))
      } else {
        if (lightSessionStartRef.current !== null) {
          const sessionSec = Math.floor((Date.now() - lightSessionStartRef.current) / 1000)
          if (sessionSec >= 30 && deviceId && lightSessionStartIsoRef.current) {
            const endedAt = new Date().toISOString()
            void api.createLightEvent(deviceId, {
              started_at: lightSessionStartIsoRef.current,
              ended_at: endedAt,
              duration_sec: sessionSec,
            }).catch(() => {})
          }
        }
        lightSessionStartRef.current = null
        lightSessionStartIsoRef.current = null
        setLightOnSeconds(0)
      }

      if (!scheduleEdited.current) {
        setLocalSchedule(
          data.feedTimes.map((ft) => ({
            ...ft,
            // firmware: -1=every day, 0=Sun, 1=Mon…6=Sat
            // UI days[]: 0=every day, 1=Mon…6=Sat, 7=Sun
            day: ft.day === -1 ? '0' : ft.day === 0 ? '7' : String(ft.day),
          }))
        )
      }

      setSAngle(data.angle)

      if (!settingsInited.current) {
        setSSpeed(Number(data.speed))
        setSDeepSleep(data.deepSleepIdleSec)
        setSDisplayEnabled(data.displayEnabled)
        setSDisplayOff(data.displayOffAfterSec)
        setSPowerSave(data.powerSaveMode)
        setSMinInterval(data.minFeedIntervalMin)
        if (!localStorage.getItem(`aq_feed_repeats_${deviceId}`)) {
          setFeedRepeats(data.feedRepeats)
        }
        settingsInited.current = true
      }

      setCooldown((prev) => {
        const backend = data.manualFeedCooldownSeconds > 0 ? data.manualFeedCooldownSeconds : 0
        return backend > prev ? backend : prev
      })
    } catch (err) {
      failCountRef.current += 1
      if (failCountRef.current >= 3) {
        setIsOnline(false)
        setError(getApiErrorMessage(err, 'Failed to load status'))
      }
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

  // ─── Feed events journal ───────────────────────────────────────────────────
  useEffect(() => {
    if (tab !== 'info' || !deviceId) return
    setFeedEventsLoading(true)
    setFeedEventsError(null)
    api.getDeviceFeedEvents(deviceId, 20)
      .then(setFeedEvents)
      .catch((err) => setFeedEventsError(getApiErrorMessage(err, 'Помилка завантаження')))
      .finally(() => setFeedEventsLoading(false))
  }, [tab, deviceId])

  // ─── Statistics ────────────────────────────────────────────────────────────
  useEffect(() => {
    if (tab !== 'stats' || !deviceId) return
    setStatsLoading(true)
    setStatsError(null)
    Promise.all([
      api.getDeviceStats(deviceId, statsDays),
      api.getLightStats(deviceId, statsDays),
    ])
      .then(([stats, light]) => {
        setDeviceStats(stats)
        setLightStats(light)
      })
      .catch((err) => setStatsError(getApiErrorMessage(err, 'Помилка завантаження')))
      .finally(() => setStatsLoading(false))
  }, [tab, deviceId, statsDays])

  // ─── Feed now ─────────────────────────────────────────────────────────────
  const handleFeed = async () => {
    if (feeding || cooldown > 0) return
    try {
      setFeeding(true)
      showToast(T.toastFeeding)
      await api.feedNow({ repeats: feedRepeats }, deviceId)
      showToast(T.toastFed)
      setCooldown(sMinInterval * 60)
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
    setScheduleDirty(true)
    setLocalSchedule((prev) => [...prev, { hour: 8, minute: 0, repeats: 1, day: '0' }])
    showToast(T.toastFeedAdded)
  }

  const removeFeedTime = (idx: number) => {
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) => prev.filter((_, i) => i !== idx))
    showToast(T.toastFeedRemoved)
  }

  const updateFeedTime = (idx: number, field: keyof LocalFeedTime, value: string | number) => {
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) =>
      prev.map((ft, i) => i === idx ? { ...ft, [field]: value } : ft)
    )
  }

  const saveSchedule = async () => {
    try {
      await api.setSchedule({
        times: localSchedule.map((ft) => ({
          hour: ft.hour,
          minute: ft.minute,
          repeats: ft.repeats,
          // UI days[]: 0=every day, 1=Mon…6=Sat, 7=Sun
          // firmware: -1=every day, 1=Mon…6=Sat, 0=Sun
          day: ft.day === '0' ? -1 : ft.day === '7' ? 0 : parseInt(ft.day),
        })),
      }, deviceId)
      setScheduleDirty(false)
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
      }, deviceId)
      setPowerDirty(false)
      showToast(T.toastSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const handlePowerSaveToggle = async (checked: boolean) => {
    setSPowerSave(checked)
    try {
      await api.setDisplaySettings({
        powerSaveMode: checked,
        deepSleepIdleSec: sDeepSleep,
        displayEnabled: sDisplayEnabled,
        displayOffAfterSec: sDisplayOff,
      }, deviceId)
      showToast(T.toastSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const handleDisplayEnabledToggle = async (checked: boolean) => {
    setSDisplayEnabled(checked)
    try {
      await api.setDisplaySettings({
        powerSaveMode: sPowerSave,
        deepSleepIdleSec: sDeepSleep,
        displayEnabled: checked,
        displayOffAfterSec: sDisplayOff,
      }, deviceId)
      showToast(T.toastSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const saveMinInterval = async () => {
    try {
      await api.setMinInterval({ minFeedIntervalMin: sMinInterval }, deviceId)
      setIntervalDirty(false)
      showToast(T.toastIntervalSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const handleOtaUpload = async () => {
    if (!otaFile) return
    setOtaState('uploading')
    setOtaError('')
    try {
      await api.otaUpdate(otaFile, deviceId)
      setOtaState('rebooting')
      let attempts = 0
      const poll = setInterval(() => {
        void (async () => {
          attempts++
          try {
            await api.getStatus(deviceId)
            clearInterval(poll)
            setOtaState('done')
            setOtaFile(null)
            void fetchStatus()
          } catch {
            if (attempts > 30) {
              clearInterval(poll)
              setOtaState('error')
              setOtaError(T.otaRebootTimeout)
            }
          }
        })()
      }, 3000)
    } catch (err) {
      setOtaState('error')
      setOtaError(getApiErrorMessage(err, T.otaFailed))
    }
  }

  const applyCalibration = async () => {
    const v = parseFloat(sCalibVoltage)
    if (isNaN(v) || v < 2.5 || v > 4.5) {
      showToast(T.calibrationInvalid)
      return
    }
    try {
      await api.calibrateBattery({ actualVoltage: v }, deviceId)
      showToast(T.toastCalibDone)
      setCalibDirty(false)
    } catch {
      showToast(T.toastCalibError)
    }
  }

  const saveTimezone = async () => {
    try {
      await api.setTimezone({ offsetHours: sTimezone }, deviceId)
      setTimezoneDirty(false)
      showToast(T.toastTimezoneSaved)
    } catch {
      showToast(T.toastSaveError)
    }
  }

  const sendAngle = useCallback((angle: number, notify: boolean) => {
    api.setAngle({ angle }, deviceId)
      .then(() => { if (notify) showToast(T.toastSaved) })
      .catch(() => { if (notify) showToast(T.toastSaveError) })
  }, [deviceId, T])

  const handleAngleChange = (value: number) => {
    setSAngle(value)
    if (angleDebounce.current) clearTimeout(angleDebounce.current)
    angleDebounce.current = setTimeout(() => {
      angleDebounce.current = null
      sendAngle(value, false)
    }, 50)
  }

  const saveSpeed = async () => {
    try {
      await api.setSpeed({ speed: sSpeed }, deviceId)
      setSpeedDirty(false)
      showToast(T.toastSaved)
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
  }

  const handleForgetWifi = async () => {
    if (!window.confirm(lang === 'uk'
      ? 'Забути мережу?\n\nПристрій повернеться в режим точки доступу. Для підключення відкрийте мережу FishFeeder-XXXX.'
      : 'Forget network?\n\nThe device will return to access point mode. Connect to FishFeeder-XXXX to reconfigure.')) return
    setForgetting(true)
    try {
      await api.forgetWifi(deviceId)
      setWifiForgetDone(true)
    } catch {
      showToast(lang === 'uk' ? 'Помилка' : 'Error')
    } finally {
      setForgetting(false)
    }
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
  const uptimeSec = status?.uptimeSeconds ?? 0
  const uptimeH = Math.floor(uptimeSec / 3600)
  const uptimeM = Math.floor((uptimeSec % 3600) / 60)

  // ─── Guard ────────────────────────────────────────────────────────────────
  if (!deviceId) {
    return <div className="error-banner">No device ID</div>
  }

  if (loading && !status) {
    return (
      <div className="aq-shell">
        <div style={{ padding: '10px 12px 0', maxWidth: 520, margin: '0 auto', width: '100%', boxSizing: 'border-box' }}>
          <span className="sk" style={{ width: 130, height: 18, display: 'block' }} />
        </div>
        <div className="aq-content">
          {/* Gauges skeleton */}
          <div className="aq-card aq-battery-card">
            <div className="aq-gauges-row">
              {[1, 2, 3].map((i) => (
                <div key={i} className="aq-gauge-wrap" style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 6 }}>
                  <span className="sk" style={{ width: '100%', aspectRatio: '260/160', borderRadius: 12 }} />
                  <span className="sk" style={{ width: '65%', height: 12 }} />
                  <span className="sk" style={{ width: '45%', height: 10 }} />
                </div>
              ))}
            </div>
            <div style={{ marginTop: 14 }}>
              <span className="sk" style={{ width: '100%', height: 8, borderRadius: 4 }} />
            </div>
          </div>
          {/* Feed button skeleton */}
          <div className="aq-card" style={{ display: 'flex', flexDirection: 'column', gap: 12 }}>
            <span className="sk" style={{ width: '45%', height: 16 }} />
            <span className="sk" style={{ width: '30%', height: 13 }} />
            <span className="sk" style={{ width: '100%', height: 52, borderRadius: 999 }} />
          </div>
          {/* Schedule skeleton */}
          <div className="aq-card" style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
            <span className="sk" style={{ width: '40%', height: 16 }} />
            <span className="sk" style={{ width: '100%', height: 56, borderRadius: 8 }} />
            <span className="sk" style={{ width: '100%', height: 56, borderRadius: 8 }} />
          </div>
        </div>
        {/* Bottom tabs skeleton */}
        <nav className="aq-bottom-tabs">
          {[1, 2, 3, 4].map((i) => (
            <div key={i} style={{ flex: 1, display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 5, padding: '10px 0' }}>
              <span className="sk" style={{ width: 22, height: 22, borderRadius: 5 }} />
              <span className="sk" style={{ width: 36, height: 9 }} />
            </div>
          ))}
        </nav>
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
          {sLightEnabled && (
            <LightGauge
              durationSeconds={lightOnSeconds}
              isOn={(status?.lightLux ?? 0) > sLightThreshold}
              T={T}
            />
          )}
        </div>

        {/* Food supply status */}
        {sFoodEnabled && foodTotalG > 0 && (
          <div className="aq-food-section">
            <div className="aq-food-header">
              <span className="aq-food-title">{T.foodTitle}</span>
            </div>
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
          </div>
        )}

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
              onClick={() => updateFeedRepeats(Math.max(1, feedRepeats - 1))}
              disabled={feedRepeats <= 1}
            >−</button>
            <span className="aq-stepper-val">{feedRepeats}</span>
            <button
              className="aq-stepper-btn"
              onClick={() => updateFeedRepeats(Math.min(10, feedRepeats + 1))}
              disabled={feedRepeats >= 10}
            >+</button>
          </div>
        </div>

        <button
          className={`aq-feed-btn${feeding ? ' is-feeding' : ''}${cooldown > 0 ? ' is-cooldown' : ''}`}
          onClick={() => void handleFeed()}
          disabled={feeding || cooldown > 0}
        >
          {feeding ? (
            T.feeding
          ) : cooldown > 0 ? (
            <>
              <span className="aq-feed-btn-cooldown-time">
                {String(Math.floor(cooldown / 60)).padStart(2, '0')}:{String(cooldown % 60).padStart(2, '0')}
              </span>
              <span className="aq-feed-btn-cooldown-label">{T.feedCooldownLabel}</span>
            </>
          ) : (
            T.feedNow
          )}
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
                <div className="aq-time-custom">
                  <select
                    className="aq-time-part"
                    value={ft.hour}
                    onChange={(e) => updateFeedTime(idx, 'hour', Number(e.target.value))}
                  >
                    {Array.from({ length: 24 }, (_, i) => (
                      <option key={i} value={i}>{String(i).padStart(2, '0')}</option>
                    ))}
                  </select>
                  <span className="aq-time-sep">:</span>
                  <select
                    className="aq-time-part"
                    value={ft.minute}
                    onChange={(e) => updateFeedTime(idx, 'minute', Number(e.target.value))}
                  >
                    {Array.from({ length: 60 }, (_, i) => (
                      <option key={i} value={i}>{String(i).padStart(2, '0')}</option>
                    ))}
                  </select>
                </div>
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
        {scheduleDirty && <button className="aq-save-btn" onClick={() => void saveSchedule()}>{T.saveAllTimes}</button>}
      </div>

      {/* Servo card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24">
              <line x1="8" y1="5" x2="8" y2="19" />
              <line x1="16" y1="5" x2="16" y2="19" />
              <circle cx="8" cy="10" r="2.5" />
              <circle cx="16" cy="14" r="2.5" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.manualControl}</div>
            <div className="aq-section-sub">{T.manualControlSub}</div>
          </div>
        </div>

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.servoAngle(sAngle)}</label>
          <input
            className="aq-servo-slider"
            type="range"
            min="0"
            max="180"
            value={sAngle}
            onChange={(e) => handleAngleChange(Number(e.target.value))}
          />
        </div>

        <div className="aq-settings-field" style={{ marginTop: 16 }}>
          <label className="aq-settings-label">{T.servoSpeed(sSpeed)}</label>
          <input
            className="aq-servo-slider"
            type="range"
            min="10"
            max="20"
            step="0.5"
            value={sSpeed}
            onChange={(e) => { setSSpeed(Number(e.target.value)); setSpeedDirty(true) }}
          />
        </div>

        {speedDirty && <button className="aq-save-btn" style={{ marginTop: 14 }} onClick={() => void saveSpeed()}>
          {T.saveSpeed}
        </button>}
      </div>
    </>
  )

  // ═══════════════════════════════════════════════════════════════════════════
  //  INFO TAB
  // ═══════════════════════════════════════════════════════════════════════════

  const toKB = (bytes: number) => `${(bytes / 1024).toFixed(2)} KB`

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

      {/* Battery card */}
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
        <div className="aq-info-row">
          <span>{T.isCharging}</span>
          <span>{status?.isCharging ? T.on : T.off}</span>
        </div>
      </div>

      {/* Settings summary card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="3" />
              <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.manualControl}</div>
            <div className="aq-section-sub">{T.manualControlSub}</div>
          </div>
        </div>
        <div className="aq-info-row">
          <span>{T.servoSpeedLabel}</span>
          <span>{status?.speed}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.feedRepeatsLabel}</span>
          <span>{feedRepeats}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.scheduleCountLabel}</span>
          <span>{localSchedule.length}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.powerSaveLabel}</span>
          <span>{status?.powerSaveMode ? T.on : T.off}</span>
        </div>
        <div className="aq-info-row">
          <span>{T.oledLabel}</span>
          <span>{status?.displayEnabled ? T.on : T.off}</span>
        </div>
      </div>

      {/* System card */}
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
        {status?.cpuFrequency != null && (
          <div className="aq-info-row">
            <span>{T.cpuFreq}</span>
            <span>{status.cpuFrequency} MHz</span>
          </div>
        )}
      </div>

      {/* Memory & Cache card */}
      {status?.memoryTotalHeap != null && (
        <div className="aq-card">
          <div className="aq-section-header">
            <div className="aq-section-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                <rect x="3" y="3" width="18" height="18" rx="2" />
                <line x1="9" y1="9" x2="15" y2="9" />
                <line x1="9" y1="12" x2="15" y2="12" />
                <line x1="9" y1="15" x2="13" y2="15" />
              </svg>
            </div>
            <div>
              <div className="aq-section-title">{T.memoryInfo}</div>
              <div className="aq-section-sub">{T.memoryInfoSub}</div>
            </div>
          </div>
          {status.memoryFreeHeap != null && (
            <div className="aq-info-row">
              <span>{T.freeMemory}</span>
              <span>{toKB(status.memoryFreeHeap)}</span>
            </div>
          )}
          {status.memoryUsedHeap != null && (
            <div className="aq-info-row">
              <span>{T.usedMemory}</span>
              <span>{toKB(status.memoryUsedHeap)}</span>
            </div>
          )}
          {status.memoryTotalHeap != null && (
            <div className="aq-info-row">
              <span>{T.totalMemory}</span>
              <span>{toKB(status.memoryTotalHeap)}</span>
            </div>
          )}
          {status.memoryMaxAllocHeap != null && (
            <div className="aq-info-row">
              <span>{T.maxBlock}</span>
              <span>{toKB(status.memoryMaxAllocHeap)}</span>
            </div>
          )}
          {status.memoryMinFreeHeap != null && (
            <div className="aq-info-row">
              <span>{T.minFree}</span>
              <span>{toKB(status.memoryMinFreeHeap)}</span>
            </div>
          )}
          {status.cacheSize != null && (
            <div className="aq-info-row">
              <span>{T.cacheSizeLabel}</span>
              <span>{toKB(status.cacheSize)}</span>
            </div>
          )}
          {status.cacheAge != null && (
            <div className="aq-info-row">
              <span>{T.cacheAgeLabel}</span>
              <span>{status.cacheAge} ms</span>
            </div>
          )}
          {status.cacheValid != null && (
            <div className="aq-info-row">
              <span>{T.cacheStatusLabel}</span>
              <span>{status.cacheValid ? T.cacheActive : T.cacheInactive}</span>
            </div>
          )}
        </div>
      )}

      {/* Feed journal card */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z" />
              <polyline points="14 2 14 8 20 8" />
              <line x1="16" y1="13" x2="8" y2="13" />
              <line x1="16" y1="17" x2="8" y2="17" />
              <polyline points="10 9 9 9 8 9" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{T.feedJournal}</div>
            <div className="aq-section-sub">{T.feedJournalSub}</div>
          </div>
        </div>
        {feedEventsError && <div className="aq-info-banner">{feedEventsError}</div>}
        {feedEventsLoading ? (
          <div style={{ display: 'flex', flexDirection: 'column', gap: 8, marginTop: 4 }}>
            {[1, 2, 3, 4, 5].map((i) => (
              <div key={i} style={{ display: 'flex', gap: 12, alignItems: 'center' }}>
                <span className="sk" style={{ width: '38%', height: 13 }} />
                <span className="sk" style={{ width: '12%', height: 13 }} />
                <span className="sk" style={{ width: '22%', height: 13 }} />
              </div>
            ))}
          </div>
        ) : feedEvents.length === 0 ? (
          <div className="aq-journal-empty">{T.feedJournalEmpty}</div>
        ) : (
          <div className="aq-journal-table-wrap">
            <table className="aq-journal-table">
              <thead>
                <tr>
                  <th>Час</th>
                  <th>Повторів</th>
                  <th>Джерело</th>
                </tr>
              </thead>
              <tbody>
                {feedEvents.map((ev) => (
                  <tr key={ev.id}>
                    <td>{new Date(ev.created_at).toLocaleString('uk-UA', { dateStyle: 'short', timeStyle: 'short' })}</td>
                    <td className="aq-journal-center">{ev.repeats}</td>
                    <td>{T.feedJournalSource(ev.source)}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
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

      {/* 3. Power settings */}
      <div className="aq-card">
        <div className="aq-section-header" style={{ alignItems: 'center', marginBottom: sPowerSave ? 14 : 0 }}>
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M18.36 6.64a9 9 0 1 1-12.73 0" />
              <line x1="12" y1="2" x2="12" y2="12" />
            </svg>
          </div>
          <div style={{ flex: 1 }}>
            <div className="aq-section-title">{T.powerSettings}</div>
            <div className="aq-section-sub">{T.powerSettingsSub}</div>
          </div>
          <label className="aq-toggle" style={{ margin: 0, width: 'auto', flexShrink: 0 }}>
            <input type="checkbox" checked={sPowerSave} onChange={(e) => { void handlePowerSaveToggle(e.target.checked) }} />
            <span className="aq-toggle-box" />
          </label>
        </div>

        {sPowerSave && (
          <div className="aq-settings-field">
            <label className="aq-settings-label">{T.deepSleepLabel}</label>
            <input
              className="aq-settings-input"
              type="number"
              min="10"
              max="3600"
              value={sDeepSleep}
              onChange={(e) => { setSDeepSleep(Number(e.target.value)); setPowerDirty(true) }}
            />
          </div>
        )}
        {powerDirty && (
          <button className="aq-save-btn" onClick={() => void savePowerSettings()}>
            {T.save}
          </button>
        )}
      </div>

      {/* 3b. OLED display */}
      <div className="aq-card">
        <div className="aq-section-header" style={{ alignItems: 'center', marginBottom: 14 }}>
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <rect x="3.5" y="5.5" width="17" height="11" rx="2" />
              <path d="M9 19h6" />
              <path d="M12 16.5v2.5" />
            </svg>
          </div>
          <div style={{ flex: 1 }}>
            <div className="aq-section-title">{T.oledTitle}</div>
            <div className="aq-section-sub">{T.oledSub}</div>
          </div>
          <label className="aq-toggle" style={{ margin: 0, width: 'auto', flexShrink: 0 }}>
            <input type="checkbox" checked={sDisplayEnabled} onChange={(e) => { void handleDisplayEnabledToggle(e.target.checked) }} />
            <span className="aq-toggle-box" />
          </label>
        </div>

        <div className="aq-settings-field">
          <label className="aq-settings-label">{T.displayOffLabel(sDisplayOff)}</label>
          <input
            className="aq-settings-input"
            type="number"
            min="5"
            max="600"
            value={sDisplayOff}
            onChange={(e) => { setSDisplayOff(Number(e.target.value)); setPowerDirty(true) }}
          />
        </div>
        {powerDirty && (
          <button className="aq-save-btn" onClick={() => void savePowerSettings()}>
            {T.save}
          </button>
        )}
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
            onChange={(e) => { setSMinInterval(Number(e.target.value)); setIntervalDirty(true) }}
          />
        </div>
        <p className="aq-settings-hint">{T.minIntervalHint(sMinInterval)}</p>
        {intervalDirty && <button className="aq-save-btn" onClick={() => void saveMinInterval()}>{T.saveInterval}</button>}
      </div>

      {/* 5. Food supply toggle */}
      <div className="aq-card">
        <div className="aq-section-header" style={{ alignItems: 'center', marginBottom: sFoodEnabled ? 14 : 0 }}>
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M3 6h18M3 12h18M3 18h18" />
            </svg>
          </div>
          <div style={{ flex: 1 }}>
            <div className="aq-section-title">{T.foodTitle}</div>
            <div className="aq-section-sub">{T.foodSettingsSub}</div>
          </div>
          <label className="aq-toggle" style={{ margin: 0, width: 'auto', flexShrink: 0 }}>
            <input
              type="checkbox"
              checked={sFoodEnabled}
              onChange={(e) => handleFoodEnabled(e.target.checked)}
            />
            <span className="aq-toggle-box" />
          </label>
        </div>

        {sFoodEnabled && (
          <div className="aq-food-form" style={{ marginTop: 0 }}>
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
            <button className="aq-food-save-btn" style={{ marginTop: 4 }} onClick={saveFoodForm}>
              {T.foodSave}
            </button>
          </div>
        )}
      </div>

      {/* 7. Light sensor */}
      <div className="aq-card">
          <div className="aq-section-header" style={{ alignItems: 'center', marginBottom: sLightEnabled ? 14 : 0 }}>
            <div className="aq-section-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                <circle cx="12" cy="12" r="5" />
                <line x1="12" y1="1" x2="12" y2="3" />
                <line x1="12" y1="21" x2="12" y2="23" />
                <line x1="4.22" y1="4.22" x2="5.64" y2="5.64" />
                <line x1="18.36" y1="18.36" x2="19.78" y2="19.78" />
                <line x1="1" y1="12" x2="3" y2="12" />
                <line x1="21" y1="12" x2="23" y2="12" />
                <line x1="4.22" y1="19.78" x2="5.64" y2="18.36" />
                <line x1="18.36" y1="5.64" x2="19.78" y2="4.22" />
              </svg>
            </div>
            <div style={{ flex: 1 }}>
              <div className="aq-section-title">{T.lightSensor}</div>
              <div className="aq-section-sub">{T.lightSettingsSub}</div>
            </div>
            <label className="aq-toggle" style={{ margin: 0, width: 'auto', flexShrink: 0 }}>
              <input
                type="checkbox"
                checked={sLightEnabled}
                onChange={(e) => handleLightEnabled(e.target.checked)}
              />
              <span className="aq-toggle-box" />
            </label>
          </div>

          {sLightEnabled && (
            <div className="aq-settings-field">
              <label className="aq-settings-label">{T.lightThresholdLabel}</label>
              <input
                className="aq-settings-input"
                type="number"
                min="1"
                max="1000"
                value={sLightThreshold}
                onChange={(e) => handleLightThreshold(Math.max(1, Math.min(1000, Number(e.target.value))))}
              />
              <p className="aq-settings-hint">{T.lightThresholdHint}</p>
            </div>
          )}
        </div>

      {/* 7. Battery calibration */}
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
            onChange={(e) => { setSCalibVoltage(e.target.value); setCalibDirty(true) }}
            placeholder="e.g. 3.742"
          />
        </div>
        <p className="aq-settings-hint">{T.calibrationHint}</p>
        {calibDirty && <button className="aq-save-btn" onClick={() => void applyCalibration()}>{T.applyCalibration}</button>}
      </div>

      {/* 8. Timezone */}
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
            onChange={(e) => { setSTimezone(Number(e.target.value)); setTimezoneDirty(true) }}
          >
            {Array.from({ length: 27 }, (_, i) => i - 12).map((offset) => (
              <option key={offset} value={offset}>
                UTC{offset >= 0 ? `+${offset}` : offset}
              </option>
            ))}
          </select>
        </div>
        <p className="aq-settings-hint">{T.timezoneHint}</p>
        {timezoneDirty && <button className="aq-save-btn" onClick={() => void saveTimezone()}>{T.saveTimezone}</button>}
      </div>

      {/* 9. OTA Firmware Update */}
      <div className="aq-card">
        {/* Header */}
        <div className="aq-section-header" style={{ alignItems: 'center', marginBottom: 16 }}>
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#4A5568" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <polyline points="16 16 12 12 8 16" />
              <line x1="12" y1="12" x2="12" y2="21" />
              <path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3" />
            </svg>
          </div>
          <div style={{ flex: 1 }}>
            <div className="aq-section-title">{T.otaTitle}</div>
            <div className="aq-section-sub">
              {T.otaCurrentVersion} <span style={{ fontWeight: 600, color: '#444' }}>{status?.firmwareVersion ?? '—'}</span>
              {(status?.buildDate || status?.buildTime) && (
                <span style={{ color: '#888', marginLeft: 6 }}>
                  ({[status.buildDate, status.buildTime].filter(Boolean).join(' ')})
                </span>
              )}
            </div>
          </div>
        </div>

        {/* Step indicator */}
        {(() => {
          const stepIndex = otaState === 'idle' ? 0 : otaState === 'uploading' ? 1 : otaState === 'rebooting' ? 1 : 2
          const steps = [
            lang === 'uk' ? 'Вибір файлу' : 'Select file',
            lang === 'uk' ? 'Завантаження' : 'Uploading',
            lang === 'uk' ? 'Готово' : 'Done',
          ]
          return (
            <div style={{ display: 'flex', alignItems: 'center', marginBottom: 18, gap: 0 }}>
              {steps.map((label, i) => {
                const active = i === stepIndex
                const completed = i < stepIndex || otaState === 'done'
                return (
                  <div key={i} style={{ display: 'flex', alignItems: 'center', flex: i < steps.length - 1 ? 1 : 'none' }}>
                    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}>
                      <div style={{
                        width: 22, height: 22, borderRadius: '50%',
                        background: completed ? '#34c759' : active ? '#111' : '#e5e7eb',
                        display: 'flex', alignItems: 'center', justifyContent: 'center', flexShrink: 0,
                        transition: 'background 0.3s',
                      }}>
                        {completed ? (
                          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="#fff" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round">
                            <polyline points="20 6 9 17 4 12" />
                          </svg>
                        ) : (
                          <span style={{ fontSize: 10, fontWeight: 700, color: active ? '#fff' : '#aaa' }}>{i + 1}</span>
                        )}
                      </div>
                      <span style={{ fontSize: 10, color: completed ? '#34c759' : active ? '#111' : '#aaa', fontWeight: active ? 600 : 400, whiteSpace: 'nowrap' }}>{label}</span>
                    </div>
                    {i < steps.length - 1 && (
                      <div style={{ flex: 1, height: 2, background: completed ? '#34c759' : '#e5e7eb', margin: '0 6px', marginBottom: 14, transition: 'background 0.3s' }} />
                    )}
                  </div>
                )
              })}
            </div>
          )
        })()}

        {/* Battery warning */}
        {status && status.batteryPercent < 20 && (
          <div style={{ display: 'flex', alignItems: 'flex-start', gap: 10, background: '#fff7ed', border: '1px solid #fed7aa', borderLeft: '3px solid #f97316', borderRadius: 8, padding: '10px 12px', marginBottom: 14 }}>
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="#f97316" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round" style={{ flexShrink: 0, marginTop: 1 }}>
              <path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z" />
              <line x1="12" y1="9" x2="12" y2="13" />
              <line x1="12" y1="17" x2="12.01" y2="17" />
            </svg>
            <span style={{ fontSize: 12, color: '#92400e', lineHeight: 1.5 }}>{T.otaBatteryWarning}</span>
          </div>
        )}

        {/* Drop zone (idle states only) */}
        {(otaState === 'idle' || otaState === 'error') && (
          <>
            <label className={`aq-ota-drop${otaFile ? ' aq-ota-drop--selected' : ''}`}>
              <input
                type="file"
                accept=".bin"
                style={{ display: 'none' }}
                onChange={(e) => { setOtaFile(e.target.files?.[0] ?? null); setOtaState('idle'); setOtaError('') }}
              />
              {otaFile ? (
                <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 6 }}>
                  <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#111" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z" />
                    <polyline points="13 2 13 9 20 9" />
                  </svg>
                  <span style={{ fontSize: 13, fontWeight: 600, color: '#111' }}>{otaFile.name}</span>
                  <span style={{ fontSize: 11, color: '#6b7280' }}>{(otaFile.size / 1024).toFixed(0)} KB — {lang === 'uk' ? 'натисніть для заміни' : 'click to replace'}</span>
                </div>
              ) : (
                <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 8 }}>
                  <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="#9ca3af" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round">
                    <polyline points="16 16 12 12 8 16" />
                    <line x1="12" y1="12" x2="12" y2="21" />
                    <path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3" />
                  </svg>
                  <span style={{ fontSize: 13, color: '#6b7280' }}>{lang === 'uk' ? 'Виберіть файл прошивки (.bin)' : 'Select firmware file (.bin)'}</span>
                </div>
              )}
            </label>

            {otaFile && otaState === 'idle' && (
              <button className="aq-save-btn" style={{ marginTop: 12, display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 8 }} onClick={() => void handleOtaUpload()}>
                <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
                  <polyline points="16 16 12 12 8 16" />
                  <line x1="12" y1="12" x2="12" y2="21" />
                  <path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3" />
                </svg>
                {T.otaFlash}
              </button>
            )}

            {otaState === 'error' && (
              <div style={{ display: 'flex', alignItems: 'center', gap: 8, marginTop: 10, color: '#ef4444' }}>
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="#ef4444" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
                  <circle cx="12" cy="12" r="10" />
                  <line x1="15" y1="9" x2="9" y2="15" />
                  <line x1="9" y1="9" x2="15" y2="15" />
                </svg>
                <span style={{ fontSize: 12 }}>{otaError}</span>
              </div>
            )}
          </>
        )}

        {/* Progress */}
        {(otaState === 'uploading' || otaState === 'rebooting') && (
          <div style={{ marginTop: 4 }}>
            <div style={{ height: 8, borderRadius: 999, background: '#e5e7eb', overflow: 'hidden' }}>
              <div
                className="aq-ota-progress-bar"
                style={{ width: otaState === 'rebooting' ? '75%' : '35%' }}
              />
            </div>
            <p className="aq-settings-hint" style={{ marginTop: 8, color: '#555' }}>
              {otaState === 'rebooting' ? T.otaRebooting : T.otaUploading}
            </p>
          </div>
        )}

        {/* Done */}
        {otaState === 'done' && (
          <div style={{ display: 'flex', alignItems: 'center', gap: 10, marginTop: 4, padding: '12px 14px', background: '#f0fdf4', border: '1px solid #bbf7d0', borderRadius: 10 }}>
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="#16a34a" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="10" />
              <polyline points="20 6 9 17 4 12" />
            </svg>
            <span style={{ fontSize: 13, color: '#15803d', fontWeight: 500 }}>{T.otaDone}</span>
          </div>
        )}
      </div>

      {/* WiFi */}
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
        <button
          type="button"
          className="aq-danger-btn"
          onClick={() => void handleForgetWifi()}
          disabled={forgetting}
        >
          {forgetting ? '…' : T.forgetNetwork}
        </button>
      </div>

      {/* 10. Account */}
      <div className="aq-card">
        <div className="aq-section-header">
          <div className="aq-section-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="#555" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" />
              <circle cx="12" cy="7" r="4" />
            </svg>
          </div>
          <div>
            <div className="aq-section-title">{accountEmail ?? T.account}</div>
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
  //  STATS TAB
  // ═══════════════════════════════════════════════════════════════════════════

  const renderStats = () => {
    const today = new Date().toISOString().slice(0, 10)

    // Today's live light: backend total + current in-progress session
    const backendTodaySec = lightStats?.days.find((d) => d.date === today)?.duration_sec ?? 0
    const todayLightSec = backendTodaySec + lightOnSeconds
    const todayLightH = Math.floor(todayLightSec / 3600)
    const todayLightM = Math.floor((todayLightSec % 3600) / 60)

    // Light sparkline — add live session to today's bar
    const lightSparkline = (lightStats?.days ?? []).map((d) => {
      const sec = d.date === today ? d.duration_sec + lightOnSeconds : d.duration_sec
      return Math.round(sec / 60)
    })

    // Shared day label helper
    const labelStep = statsDays <= 7 ? 1 : statsDays <= 14 ? 2 : 5
    const makeDayLabels = (dates: string[]) =>
      dates.map((dateStr, i, arr) => {
        const isLast = i === arr.length - 1
        if (i % labelStep !== 0 && !isLast) return ''
        const dt = new Date(dateStr + 'T12:00:00')
        return statsDays <= 7
          ? dt.toLocaleDateString('uk-UA', { weekday: 'short' })
          : dt.toLocaleDateString('uk-UA', { day: 'numeric', month: 'numeric' })
      })

    const feedingDayLabels = makeDayLabels((deviceStats?.days ?? []).map((d) => d.date))
    const lightDayLabels = makeDayLabels((lightStats?.days ?? []).map((d) => d.date))

    const condRecs: string[] = []
    const todayCount = deviceStats?.feedings_today ?? 0
    const avg = deviceStats?.avg_feedings_per_day ?? 0
    if (todayCount === 0) condRecs.push(T.recNoFeedingsToday)
    else if (avg < 2) condRecs.push(T.recFeedMore)
    if (todayCount > 5) condRecs.push(T.recFeedTooMuch)
    if (todayLightSec === 0) condRecs.push(T.recNoLight)
    else if (todayLightH < 8) condRecs.push(T.recLightTooShort)
    else condRecs.push(T.recLightGood)
    if (condRecs.length === 0 || (todayCount >= 2 && todayCount <= 4 && todayLightH >= 8)) condRecs.push(T.recAllGood)

    const dayOfYear = Math.floor((Date.now() - new Date(new Date().getFullYear(), 0, 0).getTime()) / 86400000)
    const tips = T.dailyTips
    const tip1 = tips[dayOfYear % tips.length]
    const tip2 = tips[(dayOfYear + 7) % tips.length]
    const recs = [...new Set([...condRecs, tip1, tip2])].slice(0, 5)

    return (
      <>
        {statsError && <div className="aq-info-banner">{statsError}</div>}
        {statsLoading && (
          <>
            <div className="aq-stats-grid">
              {[1, 2, 3].map((i) => (
                <div key={i} className="aq-stats-card" style={{ gap: 8 }}>
                  <span className="sk" style={{ width: '60%', height: 26 }} />
                  <span className="sk" style={{ width: '80%', height: 11 }} />
                </div>
              ))}
            </div>
            <div className="aq-card" style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
              <span className="sk" style={{ width: '50%', height: 14 }} />
              <span className="sk" style={{ width: '100%', height: 40, borderRadius: 4 }} />
              <span className="sk" style={{ width: '100%', height: 12 }} />
            </div>
            <div className="aq-card" style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
              <span className="sk" style={{ width: '50%', height: 14 }} />
              <span className="sk" style={{ width: '100%', height: 40, borderRadius: 4 }} />
              <span className="sk" style={{ width: '100%', height: 12 }} />
            </div>
          </>
        )}

        {/* Summary cards */}
        <div className="aq-stats-grid">
          <div className="aq-stats-card">
            <span className="aq-stats-value">{deviceStats?.feedings_today ?? '--'}</span>
            <span className="aq-stats-label">{T.statsFeedingsToday}</span>
          </div>
          <div className="aq-stats-card">
            <span className="aq-stats-value">{deviceStats?.avg_feedings_per_day ?? '--'}</span>
            <span className="aq-stats-label">{T.statsAvgPerDay}</span>
          </div>
          <div className="aq-stats-card">
            <span className="aq-stats-value">{todayLightH > 0 || todayLightM > 0 ? T.statsLightHours(todayLightH, todayLightM) : '--'}</span>
            <span className="aq-stats-label">{T.statsLightToday}</span>
          </div>
        </div>

        {/* Shared period selector */}
        <div className="aq-stats-period-bar">
          {([7, 14, 30] as const).map((d) => (
            <button
              key={d}
              className={`aq-period-pill${statsDays === d ? ' active' : ''}`}
              onClick={() => setStatsDays(d)}
            >
              {d}д
            </button>
          ))}
        </div>

        {/* Feedings chart */}
        <div className="aq-card">
          <div className="aq-stats-section-title">{T.statsFeedings}</div>
          <Sparkline data={deviceStats?.feedings_sparkline ?? []} id="dev-feedings" color="#1976D2" />
          <div className="aq-stats-days-row">
            {feedingDayLabels.map((label, i) => <span key={i}>{label}</span>)}
          </div>
        </div>

        {/* Light chart */}
        {status?.lightLux != null && (
          <div className="aq-card">
            <div className="aq-stats-section-title">{T.statsLight}</div>
            <Sparkline data={lightSparkline} id="dev-light" color="#D97706" />
            <div className="aq-stats-days-row">
              {lightDayLabels.map((label, i) => <span key={i}>{label}</span>)}
            </div>
          </div>
        )}

        {/* Recommendations */}
        <div className="aq-card">
          <div className="aq-stats-section-title">{T.statsRecommendations}</div>
          <ul className="aq-recs-list">
            {recs.map((r, i) => <li key={i} className="aq-rec-item">{r}</li>)}
          </ul>
        </div>
      </>
    )
  }

  // ═══════════════════════════════════════════════════════════════════════════
  //  RENDER
  // ═══════════════════════════════════════════════════════════════════════════

  return (
    <div className="aq-shell">
      {/* WiFi forget modal */}
      {wifiForgetDone && (
        <div className="aq-wifi-modal-overlay">
          <div className="aq-wifi-modal">
            <div className="aq-wifi-modal-title">
              {lang === 'uk' ? 'Мережу забуто' : 'Network forgotten'}
            </div>
            <p className="aq-wifi-modal-note">
              {lang === 'uk'
                ? 'Всі налаштування збережено. Щоб підключитись до нової мережі:'
                : 'All settings are preserved. To connect to a new network:'}
            </p>
            <div className="aq-wifi-modal-step">
              {lang === 'uk' ? '1. Підключіться до WiFi:' : '1. Connect to WiFi:'}
            </div>
            <div className="aq-wifi-modal-code">FishFeeder-XXXX</div>
            <div className="aq-wifi-modal-step" style={{ marginTop: -4 }}>
              {lang === 'uk' ? 'Пароль:' : 'Password:'} <strong>12345678</strong>
            </div>
            <div className="aq-wifi-modal-step" style={{ marginTop: 10 }}>
              {lang === 'uk' ? '2. Відкрийте в браузері:' : '2. Open in browser:'}
            </div>
            <div className="aq-wifi-modal-code">192.168.4.1</div>
            <div className="aq-wifi-modal-step">
              {lang === 'uk' ? '3. Введіть нові дані WiFi' : '3. Enter new WiFi credentials'}
            </div>
            <button
              type="button"
              className="aq-wifi-modal-btn"
              onClick={() => { setWifiForgetDone(false); navigate('/devices') }}
            >
              {lang === 'uk' ? 'Зрозуміло' : 'Got it'}
            </button>
          </div>
        </div>
      )}

      {/* Offline overlay */}
      {isOnline === false && !loading && (
        <div className="aq-offline-overlay">
          <div className="aq-offline-card">
            <div className="aq-offline-icon">
              <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round">
                <line x1="1" y1="1" x2="23" y2="23"/>
                <path d="M16.72 11.06A10.94 10.94 0 0 1 19 12.55"/>
                <path d="M5 12.55a10.94 10.94 0 0 1 5.17-2.39"/>
                <path d="M10.71 5.05A16 16 0 0 1 22.56 9"/>
                <path d="M1.42 9a15.91 15.91 0 0 1 4.7-2.88"/>
                <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/>
                <line x1="12" y1="20" x2="12.01" y2="20"/>
              </svg>
            </div>
            <div className="aq-offline-title">
              {lang === 'uk' ? 'Пристрій офлайн' : 'Device offline'}
            </div>
            <p className="aq-offline-desc">
              {lang === 'uk'
                ? 'Годівниця недоступна. Якщо ви щойно забули мережу — пристрій очікує підключення до нового WiFi.'
                : 'Feeder is unreachable. If you just forgot the network — the device is waiting for new WiFi credentials.'}
            </p>
            <div className="aq-offline-steps">
              <div className="aq-offline-step">
                <span className="aq-offline-step-num">1</span>
                <span>{lang === 'uk' ? 'Підключіться до WiFi:' : 'Connect to WiFi:'} <strong>FishFeeder-XXXX</strong> {lang === 'uk' ? '(пароль:' : '(password:'} <strong>12345678</strong>)</span>
              </div>
              <div className="aq-offline-step">
                <span className="aq-offline-step-num">2</span>
                <span>{lang === 'uk' ? 'Відкрийте в браузері:' : 'Open in browser:'} <strong>192.168.4.1</strong></span>
              </div>
              <div className="aq-offline-step">
                <span className="aq-offline-step-num">3</span>
                <span>{lang === 'uk' ? 'Введіть нові дані WiFi' : 'Enter new WiFi credentials'}</span>
              </div>
            </div>
            <div className="aq-offline-actions">
              <button type="button" className="aq-offline-retry" onClick={() => void fetchStatus()}>
                {lang === 'uk' ? 'Перевірити підключення' : 'Check connection'}
              </button>
              <button type="button" className="aq-offline-back" onClick={() => navigate('/devices')}>
                {lang === 'uk' ? 'До списку' : 'Back to list'}
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Toast */}
      <div className={`aq-toast${toastVisible ? ' show' : ''}`}>
        <span className="aq-toast-icon">i</span>
        {toastMsg}
      </div>

      {/* Device mini-header */}
      <div style={{ padding: '10px 12px 0', maxWidth: 520, margin: '0 auto', width: '100%', boxSizing: 'border-box' }}>
        {device && (
          <div style={{ marginBottom: 4 }}>
            <div style={{ fontSize: 17, fontWeight: 700, color: '#1f2937' }}>{device.name}</div>
          </div>
        )}
        {error && <div className="error-banner">{error}</div>}
      </div>

      {/* Scrollable content */}
      <div className="aq-content">
        {tab === 'home' && renderHome()}
        {tab === 'info' && renderInfo()}
        {tab === 'settings' && renderSettings()}
        {tab === 'stats' && renderStats()}
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
          className={`aq-tab${tab === 'stats' ? ' active' : ''}`}
          onClick={() => setTab('stats')}
        >
          <StatsIcon />
          {T.tabStats}
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
