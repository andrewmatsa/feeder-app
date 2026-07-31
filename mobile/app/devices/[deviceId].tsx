import Slider from '@react-native-community/slider'
import * as DocumentPicker from 'expo-document-picker'
import { Redirect, useLocalSearchParams, useRouter } from 'expo-router'
import * as SecureStore from 'expo-secure-store'
import { useCallback, useEffect, useRef, useState } from 'react'
import {
  ActivityIndicator,
  Alert,
  Modal,
  Pressable,
  RefreshControl,
  ScrollView,
  StyleSheet,
  Switch,
  Text,
  TextInput,
  View,
} from 'react-native'
import { useSafeAreaInsets } from 'react-native-safe-area-context'
import { AquariumScreen } from '../../src/components/AquariumScreen'
import { FeedingCelebration } from '../../src/components/FeedingCelebration'
import { FishLogo } from '../../src/components/FishLogo'
import { BatteryGauge, LightGauge, NextFeedGauge } from '../../src/components/Gauges'
import { SectionIcon, type SectionIconName } from '../../src/components/SectionIcon'
import { Sparkline } from '../../src/components/Sparkline'
import { HomeIcon, InfoIcon, SettingsIcon, StatsIcon } from '../../src/components/TabIcons'
import { api, getApiErrorMessage } from '../../src/services/api'
import { useAuthStore } from '../../src/store/authStore'
import { type Lang, type Translation, TRANSLATIONS } from '../../src/translations'
import type { DeviceFeedEvent, DeviceStats, LightStats, StatusResponse } from '../../src/types'

type Tab = 'home' | 'info' | 'settings' | 'stats'

interface LocalFeedTime {
  hour: number
  minute: number
  repeats: number
  day: string // UI select value: '0'=every day, '1'=Mon…'6'=Sat, '7'=Sun
}

const LANG_KEY = 'aquafeed_lang'
const lightEnabledKey = (deviceId: string) => `aq_light_enabled_${deviceId}`
const lightThresholdKey = (deviceId: string) => `aq_light_threshold_${deviceId}`
const foodEnabledKey = (deviceId: string) => `aq_food_enabled_${deviceId}`
// Note: total/loadedAt/gramsPerFeed are NOT per-device — matches web's
// (accidentally) shared-across-devices localStorage keys; porting as-is for parity.
const FOOD_TOTAL_KEY = 'aq_food_total'
const FOOD_TS_KEY = 'aq_food_ts'
const FOOD_GPF_KEY = 'aq_food_gpf'

function calcFoodRemaining(totalG: number, loadedTs: number, gpf: number, feedTimesPerDay: number): number {
  if (!totalG || !loadedTs || !gpf || !feedTimesPerDay) return totalG
  const elapsedDays = (Date.now() - loadedTs) / 1000 / 3600 / 24
  const consumed = elapsedDays * feedTimesPerDay * gpf
  return Math.max(0, totalG - consumed)
}

function calcFoodDuration(remainingG: number, gpf: number, feedTimesPerDay: number): { months: number; days: number } {
  if (!gpf || !feedTimesPerDay || !remainingG) return { months: 0, days: 0 }
  const totalDays = remainingG / (gpf * feedTimesPerDay)
  const months = Math.floor(totalDays / 30)
  const days = Math.round(totalDays % 30)
  return { months, days }
}

const toKB = (bytes: number) => `${(bytes / 1024).toFixed(2)} KB`

export default function DeviceDashboardScreen() {
  const { deviceId } = useLocalSearchParams<{ deviceId: string }>()
  const router = useRouter()
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const insets = useSafeAreaInsets()

  const [lang, setLangState] = useState<Lang>('uk')
  const T = TRANSLATIONS[lang]

  useEffect(() => {
    SecureStore.getItemAsync(LANG_KEY).then((saved) => {
      if (saved === 'uk' || saved === 'en') setLangState(saved)
    })
  }, [])

  const setLang = (l: Lang) => {
    setLangState(l)
    void SecureStore.setItemAsync(LANG_KEY, l)
  }

  useEffect(() => {
    if (!deviceId) return
    api.getDevice(deviceId).then((d) => setDeviceName(d.name)).catch(() => {})
  }, [deviceId])

  const [tab, setTab] = useState<Tab>('home')
  const [deviceName, setDeviceName] = useState<string | null>(null)
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [refreshing, setRefreshing] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const [isOnline, setIsOnline] = useState<boolean | null>(null)
  const failCountRef = useRef(0)

  const [feeding, setFeeding] = useState(false)
  const [showFeedCelebration, setShowFeedCelebration] = useState(false)
  const [feedRepeats, setFeedRepeats] = useState(1)
  const [cooldown, setCooldown] = useState(0)
  const [feedError, setFeedError] = useState<string | null>(null)

  const [feedEvents, setFeedEvents] = useState<DeviceFeedEvent[]>([])
  const [feedEventsLoading, setFeedEventsLoading] = useState(false)

  // ── Stats tab ────────────────────────────────────────────────────────────
  const [deviceStats, setDeviceStats] = useState<DeviceStats | null>(null)
  const [lightStats, setLightStats] = useState<LightStats | null>(null)
  const [statsLoading, setStatsLoading] = useState(false)
  const [statsError, setStatsError] = useState<string | null>(null)
  const [statsDays, setStatsDays] = useState<7 | 14 | 30>(7)

  // ── Schedule editing (mirrors web's LocalFeedTime; UI day: 0=every day, 1-6=Mon-Sat, 7=Sun) ──
  const scheduleEdited = useRef(false)
  const [localSchedule, setLocalSchedule] = useState<LocalFeedTime[]>([])
  const [scheduleDirty, setScheduleDirty] = useState(false)

  // ── Servo (angle sends immediately, debounced; speed needs explicit save) ──
  const [sAngle, setSAngle] = useState(90)
  const [sSpeed, setSSpeed] = useState(20)
  const [speedDirty, setSpeedDirty] = useState(false)
  const angleDebounce = useRef<ReturnType<typeof setTimeout> | null>(null)

  // ── Light sensor (settings persisted per-device; hardware note: GPIO5/LDR is
  // disabled in firmware on ESP32-C3 per CLAUDE.md — this mirrors the web UI
  // for parity, but lightLux will read as inactive on this hardware) ────────
  const [sLightEnabled, setSLightEnabledState] = useState(true)
  const [sLightThreshold, setSLightThresholdState] = useState(10)
  const [lightOnSeconds, setLightOnSeconds] = useState(0)
  const lightSessionStartRef = useRef<number | null>(null)
  const lightSessionStartIsoRef = useRef<string | null>(null)

  useEffect(() => {
    if (!deviceId) return
    SecureStore.getItemAsync(lightEnabledKey(deviceId)).then((v) => {
      if (v != null) setSLightEnabledState(v !== 'false')
    })
    SecureStore.getItemAsync(lightThresholdKey(deviceId)).then((v) => {
      if (v != null) setSLightThresholdState(Number(v))
    })
  }, [deviceId])

  const setSLightEnabled = (v: boolean) => {
    setSLightEnabledState(v)
    if (deviceId) void SecureStore.setItemAsync(lightEnabledKey(deviceId), String(v))
  }

  const setSLightThreshold = (v: number) => {
    setSLightThresholdState(v)
    if (deviceId) void SecureStore.setItemAsync(lightThresholdKey(deviceId), String(v))
  }

  // ── Food supply (client-side estimate only, no backend — matches web) ─────
  const [sFoodEnabled, setSFoodEnabledState] = useState(true)
  const [foodTotalG, setFoodTotalG] = useState(0)
  const [foodLoadedTs, setFoodLoadedTs] = useState(0)
  const [gramsPerFeed, setGramsPerFeed] = useState(0)
  const [foodInputG, setFoodInputG] = useState('')
  const [foodInputGpf, setFoodInputGpf] = useState('')

  useEffect(() => {
    if (!deviceId) return
    SecureStore.getItemAsync(foodEnabledKey(deviceId)).then((v) => {
      if (v != null) setSFoodEnabledState(v !== 'false')
    })
    SecureStore.getItemAsync(FOOD_TOTAL_KEY).then((v) => {
      if (v != null) {
        setFoodTotalG(Number(v))
        setFoodInputG(v)
      }
    })
    SecureStore.getItemAsync(FOOD_TS_KEY).then((v) => {
      if (v != null) setFoodLoadedTs(Number(v))
    })
    SecureStore.getItemAsync(FOOD_GPF_KEY).then((v) => {
      if (v != null) {
        setGramsPerFeed(Number(v))
        setFoodInputGpf(v)
      }
    })
  }, [deviceId])

  const setSFoodEnabled = (v: boolean) => {
    setSFoodEnabledState(v)
    if (deviceId) void SecureStore.setItemAsync(foodEnabledKey(deviceId), String(v))
  }

  const saveFoodForm = () => {
    const g = parseFloat(foodInputG)
    const gpf = parseFloat(foodInputGpf)
    if (!isNaN(g) && g > 0) {
      const ts = Date.now()
      setFoodTotalG(g)
      setFoodLoadedTs(ts)
      void SecureStore.setItemAsync(FOOD_TOTAL_KEY, String(g))
      void SecureStore.setItemAsync(FOOD_TS_KEY, String(ts))
    }
    if (!isNaN(gpf) && gpf > 0) {
      setGramsPerFeed(gpf)
      void SecureStore.setItemAsync(FOOD_GPF_KEY, String(gpf))
    }
    showSettingsToast(T.toastSaved)
  }

  // ── Settings state (initialized from status on first load) ────────────────
  const settingsInited = useRef(false)
  const [sDeepSleep, setSDeepSleep] = useState(300)
  const [sDisplayEnabled, setSDisplayEnabled] = useState(true)
  const [sDisplayOff, setSDisplayOff] = useState(30)
  const [sPowerSave, setSPowerSave] = useState(false)
  const [powerDirty, setPowerDirty] = useState(false)
  const [sMinInterval, setSMinInterval] = useState(5)
  const [intervalDirty, setIntervalDirty] = useState(false)
  const [sCalibVoltage, setSCalibVoltage] = useState('')
  const [calibDirty, setCalibDirty] = useState(false)
  const [sTimezone, setSTimezone] = useState(2)
  const [timezoneDirty, setTimezoneDirty] = useState(false)
  const [settingsToast, setSettingsToast] = useState<string | null>(null)
  const [otaFile, setOtaFile] = useState<DocumentPicker.DocumentPickerAsset | null>(null)
  const [otaState, setOtaState] = useState<'idle' | 'uploading' | 'rebooting' | 'done' | 'error'>('idle')
  const [otaError, setOtaError] = useState('')
  const [forgetting, setForgetting] = useState(false)
  const [wifiForgetDone, setWifiForgetDone] = useState(false)

  const showSettingsToast = useCallback((msg: string) => {
    setSettingsToast(msg)
    setTimeout(() => setSettingsToast(null), 2200)
  }, [])

  const fetchStatus = useCallback(async () => {
    if (!deviceId) return
    try {
      const data = await api.getStatus(deviceId)
      failCountRef.current = 0
      setIsOnline(true)
      setStatus(data)
      setError(null)

      setSAngle(data.angle)

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
            void api
              .createLightEvent(deviceId, {
                started_at: lightSessionStartIsoRef.current,
                ended_at: endedAt,
                duration_sec: sessionSec,
              })
              .catch(() => {})
          }
        }
        lightSessionStartRef.current = null
        lightSessionStartIsoRef.current = null
        setLightOnSeconds(0)
      }

      if (!settingsInited.current) {
        setSSpeed(Number(data.speed))
        setSDeepSleep(data.deepSleepIdleSec)
        setSDisplayEnabled(data.displayEnabled)
        setSDisplayOff(data.displayOffAfterSec)
        setSPowerSave(data.powerSaveMode)
        setSMinInterval(data.minFeedIntervalMin)
        settingsInited.current = true
      }

      if (!scheduleEdited.current) {
        setLocalSchedule(
          data.feedTimes.map((ft) => ({
            ...ft,
            // firmware: -1=every day, 0=Sun, 1=Mon…6=Sat
            // UI days[]: 0=every day, 1=Mon…6=Sat, 7=Sun
            day: ft.day === -1 ? '0' : ft.day === 0 ? '7' : String(ft.day),
          })),
        )
      }

      setCooldown((prev) => {
        const backend = data.manualFeedCooldownSeconds > 0 ? data.manualFeedCooldownSeconds : 0
        return backend > prev ? backend : prev
      })
    } catch (err) {
      failCountRef.current += 1
      if (failCountRef.current >= 3) {
        setIsOnline(false)
        setError(getApiErrorMessage(err, T.loadError))
      }
    } finally {
      setLoading(false)
      setRefreshing(false)
    }
  }, [deviceId, T.loadError, sLightThreshold])

  useEffect(() => {
    void fetchStatus()
    const interval = setInterval(() => void fetchStatus(), 5000)
    return () => clearInterval(interval)
  }, [fetchStatus])

  useEffect(() => {
    if (cooldown <= 0) return
    const t = setInterval(() => {
      setCooldown((c) => (c <= 1 ? 0 : c - 1))
    }, 1000)
    return () => clearInterval(t)
  }, [cooldown])

  useEffect(() => {
    if (tab !== 'info' || !deviceId) return
    setFeedEventsLoading(true)
    api
      .getDeviceFeedEvents(deviceId, 20)
      .then(setFeedEvents)
      .catch(() => {})
      .finally(() => setFeedEventsLoading(false))
  }, [tab, deviceId])

  useEffect(() => {
    if (tab !== 'stats' || !deviceId) return
    setStatsLoading(true)
    setStatsError(null)
    Promise.all([api.getDeviceStats(deviceId, statsDays), api.getLightStats(deviceId, statsDays)])
      .then(([stats, light]) => {
        setDeviceStats(stats)
        setLightStats(light)
      })
      .catch((err) => setStatsError(getApiErrorMessage(err, T.loadError)))
      .finally(() => setStatsLoading(false))
  }, [tab, deviceId, statsDays, T.loadError])

  if (!isAuthenticated) {
    return <Redirect href="/login" />
  }

  if (!deviceId) {
    return (
      <AquariumScreen interactive>
        <View style={styles.center}>
          <Text style={styles.centerText}>No device ID</Text>
        </View>
      </AquariumScreen>
    )
  }

  const handleFeed = async () => {
    if (feeding || cooldown > 0 || isOnline === false) return
    setFeeding(true)
    setFeedError(null)
    try {
      await api.feedNow({ repeats: feedRepeats }, deviceId)
      setCooldown(status?.minFeedIntervalMin ? status.minFeedIntervalMin * 60 : 300)
      setShowFeedCelebration(true)
      setTimeout(() => setShowFeedCelebration(false), 3000)
      setTimeout(() => {
        setFeeding(false)
        void fetchStatus()
      }, 1500)
    } catch (err) {
      setFeedError(getApiErrorMessage(err, T.toastFeedError))
      setFeeding(false)
    }
  }

  const onRefresh = () => {
    setRefreshing(true)
    void fetchStatus()
  }

  // ── Schedule handlers ────────────────────────────────────────────────────
  const addFeedTime = () => {
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) => [...prev, { hour: 8, minute: 0, repeats: 1, day: '0' }])
    showSettingsToast(T.toastFeedAdded)
  }

  const removeFeedTime = (idx: number) => {
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) => prev.filter((_, i) => i !== idx))
    showSettingsToast(T.toastFeedRemoved)
  }

  const updateFeedTime = (idx: number, field: keyof LocalFeedTime, value: string | number) => {
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) => prev.map((ft, i) => (i === idx ? { ...ft, [field]: value } : ft)))
  }

  // Firmware/backend only support one day per schedule entry (same as web).
  // Picking multiple days (e.g. Mon+Wed+Fri) in the UI clones this entry's
  // time/repeats into a separate row per extra day chosen.
  const applyDaysToFeedTime = (idx: number, selectedDays: string[]) => {
    if (selectedDays.length === 0) return
    scheduleEdited.current = true
    setScheduleDirty(true)
    setLocalSchedule((prev) => {
      const base = prev[idx]
      if (!base) return prev
      const [firstDay, ...restDays] = selectedDays
      const next = [...prev]
      next[idx] = { ...base, day: firstDay }
      const clones = restDays.map((day) => ({ hour: base.hour, minute: base.minute, repeats: base.repeats, day }))
      next.splice(idx + 1, 0, ...clones)
      return next
    })
  }

  const saveSchedule = async () => {
    if (!deviceId) return
    try {
      await api.setSchedule({
        times: localSchedule.map((ft) => ({
          hour: ft.hour,
          minute: ft.minute,
          repeats: ft.repeats,
          // UI days[]: 0=every day, 1=Mon…6=Sat, 7=Sun
          // firmware: -1=every day, 1=Mon…6=Sat, 0=Sun
          day: ft.day === '0' ? -1 : ft.day === '7' ? 0 : parseInt(ft.day, 10),
        })),
      }, deviceId)
      setScheduleDirty(false)
      showSettingsToast(T.toastScheduleSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
    }
  }

  // ── Servo ────────────────────────────────────────────────────────────────
  const sendAngle = useCallback(
    (angle: number) => {
      if (!deviceId) return
      void api.setAngle({ angle }, deviceId).catch(() => {})
    },
    [deviceId],
  )

  const handleAngleChange = (value: number) => {
    setSAngle(value)
    if (angleDebounce.current) clearTimeout(angleDebounce.current)
    angleDebounce.current = setTimeout(() => {
      angleDebounce.current = null
      sendAngle(value)
    }, 50)
  }

  const saveSpeed = async () => {
    if (!deviceId) return
    try {
      await api.setSpeed({ speed: sSpeed }, deviceId)
      setSpeedDirty(false)
      showSettingsToast(T.toastSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
    }
  }

  // ── Settings handlers ───────────────────────────────────────────────────
  const savePowerSettings = async () => {
    try {
      await api.setDisplaySettings({
        powerSaveMode: sPowerSave,
        deepSleepIdleSec: sDeepSleep,
        displayEnabled: sDisplayEnabled,
        displayOffAfterSec: sDisplayOff,
      }, deviceId)
      setPowerDirty(false)
      showSettingsToast(T.toastSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
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
      showSettingsToast(T.toastSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
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
      showSettingsToast(T.toastSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
    }
  }

  const saveMinInterval = async () => {
    try {
      await api.setMinInterval({ minFeedIntervalMin: sMinInterval }, deviceId)
      setIntervalDirty(false)
      showSettingsToast(T.toastIntervalSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
    }
  }

  const applyCalibration = async () => {
    const v = parseFloat(sCalibVoltage)
    if (isNaN(v) || v < 2.5 || v > 4.5) {
      showSettingsToast(T.calibrationInvalid)
      return
    }
    try {
      await api.calibrateBattery({ actualVoltage: v }, deviceId)
      showSettingsToast(T.toastCalibDone)
      setCalibDirty(false)
    } catch {
      showSettingsToast(T.toastCalibError)
    }
  }

  const saveTimezone = async () => {
    try {
      await api.setTimezone({ offsetHours: sTimezone }, deviceId)
      setTimezoneDirty(false)
      showSettingsToast(T.toastTimezoneSaved)
    } catch {
      showSettingsToast(T.toastSaveError)
    }
  }

  const pickOtaFile = async () => {
    const result = await DocumentPicker.getDocumentAsync({ type: 'application/octet-stream', copyToCacheDirectory: true })
    if (result.canceled || result.assets.length === 0) return
    setOtaFile(result.assets[0])
    setOtaState('idle')
    setOtaError('')
  }

  const handleOtaUpload = async () => {
    if (!otaFile) return
    setOtaState('uploading')
    setOtaError('')
    try {
      await api.otaUpdate({ uri: otaFile.uri, name: otaFile.name, mimeType: otaFile.mimeType }, deviceId)
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

  const handleForgetWifi = () => {
    Alert.alert(T.forgetWifiConfirmTitle, T.forgetWifiConfirmBody, [
      { text: T.cancel, style: 'cancel' },
      {
        text: T.forgetWifi,
        style: 'destructive',
        onPress: async () => {
          setForgetting(true)
          try {
            await api.forgetWifi(deviceId)
            setWifiForgetDone(true)
          } catch {
            showSettingsToast(T.toastSaveError)
          } finally {
            setForgetting(false)
          }
        },
      },
    ])
  }

  const isLightOn = (status?.lightLux ?? 0) > sLightThreshold

  // ── Food calculations ───────────────────────────────────────────────────
  const feedTimesPerDay = localSchedule.filter((ft) => ft.day === '0').length || localSchedule.length
  const foodRemaining = foodTotalG > 0 ? calcFoodRemaining(foodTotalG, foodLoadedTs, gramsPerFeed, feedTimesPerDay || 1) : 0
  const foodPercent = foodTotalG > 0 ? Math.min(100, (foodRemaining / foodTotalG) * 100) : 0
  const foodBarColor = foodPercent > 50 ? '#4CAF50' : foodPercent > 20 ? '#FF9800' : '#f44336'
  const { months: foodMonths, days: foodDays } =
    foodTotalG > 0 && gramsPerFeed > 0 ? calcFoodDuration(foodRemaining, gramsPerFeed, feedTimesPerDay || 1) : { months: 0, days: 0 }

  if (loading && !status) {
    return (
      <AquariumScreen interactive>
        <View style={styles.center}>
          <ActivityIndicator size="large" color="#fff" />
        </View>
      </AquariumScreen>
    )
  }

  if (wifiForgetDone) {
    return (
      <AquariumScreen interactive>
        <View style={styles.center}>
          <Text style={styles.centerTitle}>{T.forgetWifiDone}</Text>
          <Pressable style={[styles.feedButton, { marginTop: 20, paddingHorizontal: 24 }]} onPress={() => router.replace('/devices')}>
            <Text style={styles.feedButtonText}>{T.backToDevices}</Text>
          </Pressable>
        </View>
      </AquariumScreen>
    )
  }

  return (
    <AquariumScreen interactive>
      <View style={[styles.headerBar, { paddingTop: insets.top + 8 }]}>
        <Pressable style={styles.backButton} onPress={() => router.back()} hitSlop={8}>
          <Text style={styles.backText}>←</Text>
        </Pressable>
        <View style={styles.brand}>
          <FishLogo size={32} />
          <View style={styles.brandTextCol}>
            <Text style={styles.brandTitle} numberOfLines={1}>{deviceName ?? '…'}</Text>
            <Text style={styles.brandSubtitle}>{tab === 'home' ? T.tabHomeSubtitle : tab === 'info' ? T.tabInfo : T.tabSettings}</Text>
          </View>
        </View>
        <View style={styles.headerSpacer} />
      </View>

      {settingsToast && (
        <View style={styles.toast}>
          <Text style={styles.toastText}>{settingsToast}</Text>
        </View>
      )}

      {isOnline === false && (
        <View style={styles.offlineBanner}>
          <Text style={styles.offlineBannerText}>{T.offline}</Text>
        </View>
      )}
      {error && <Text style={styles.error}>{error}</Text>}

      <ScrollView
        contentContainerStyle={styles.content}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
      >
        {tab === 'home' && status && (
          <HomeTab
            status={status}
            T={T}
            feeding={feeding}
            feedRepeats={feedRepeats}
            setFeedRepeats={setFeedRepeats}
            cooldown={cooldown}
            feedError={feedError}
            isOnline={isOnline}
            onFeed={() => void handleFeed()}
            localSchedule={localSchedule}
            scheduleDirty={scheduleDirty}
            onAddFeedTime={addFeedTime}
            onRemoveFeedTime={removeFeedTime}
            onUpdateFeedTime={updateFeedTime}
            onApplyDays={applyDaysToFeedTime}
            onSaveSchedule={() => void saveSchedule()}
            sAngle={sAngle}
            onAngleChange={handleAngleChange}
            sSpeed={sSpeed}
            speedDirty={speedDirty}
            onSpeedChange={(v) => {
              setSSpeed(v)
              setSpeedDirty(true)
            }}
            onSaveSpeed={() => void saveSpeed()}
            sLightEnabled={sLightEnabled}
            lightOnSeconds={lightOnSeconds}
            isLightOn={isLightOn}
            sFoodEnabled={sFoodEnabled}
            foodTotalG={foodTotalG}
            foodRemaining={foodRemaining}
            foodPercent={foodPercent}
            foodBarColor={foodBarColor}
            gramsPerFeed={gramsPerFeed}
            feedTimesPerDay={feedTimesPerDay}
            foodMonths={foodMonths}
            foodDays={foodDays}
          />
        )}

        {tab === 'info' && status && (
          <InfoTab status={status} T={T} feedEvents={feedEvents} feedEventsLoading={feedEventsLoading} />
        )}

        {tab === 'settings' && status && (
          <SettingsTab
            status={status}
            T={T}
            lang={lang}
            setLang={setLang}
            sPowerSave={sPowerSave}
            sDeepSleep={sDeepSleep}
            setSDeepSleep={(v) => { setSDeepSleep(v); setPowerDirty(true) }}
            powerDirty={powerDirty}
            onPowerSaveToggle={(v) => void handlePowerSaveToggle(v)}
            onSavePowerSettings={() => void savePowerSettings()}
            sDisplayEnabled={sDisplayEnabled}
            onDisplayEnabledToggle={(v) => void handleDisplayEnabledToggle(v)}
            sDisplayOff={sDisplayOff}
            setSDisplayOff={(v) => { setSDisplayOff(v); setPowerDirty(true) }}
            sMinInterval={sMinInterval}
            setSMinInterval={(v) => { setSMinInterval(v); setIntervalDirty(true) }}
            intervalDirty={intervalDirty}
            onSaveInterval={() => void saveMinInterval()}
            sCalibVoltage={sCalibVoltage}
            setSCalibVoltage={(v) => { setSCalibVoltage(v); setCalibDirty(true) }}
            calibDirty={calibDirty}
            onApplyCalibration={() => void applyCalibration()}
            sTimezone={sTimezone}
            setSTimezone={(v) => { setSTimezone(v); setTimezoneDirty(true) }}
            timezoneDirty={timezoneDirty}
            onSaveTimezone={() => void saveTimezone()}
            otaFile={otaFile}
            otaState={otaState}
            otaError={otaError}
            onPickOtaFile={() => void pickOtaFile()}
            onOtaUpload={() => void handleOtaUpload()}
            forgetting={forgetting}
            onForgetWifi={handleForgetWifi}
            sLightEnabled={sLightEnabled}
            setSLightEnabled={setSLightEnabled}
            sLightThreshold={sLightThreshold}
            setSLightThreshold={setSLightThreshold}
            sFoodEnabled={sFoodEnabled}
            setSFoodEnabled={setSFoodEnabled}
            foodInputG={foodInputG}
            setFoodInputG={setFoodInputG}
            foodInputGpf={foodInputGpf}
            setFoodInputGpf={setFoodInputGpf}
            onSaveFood={saveFoodForm}
          />
        )}

        {tab === 'stats' && (
          <StatsTab
            T={T}
            status={status}
            deviceStats={deviceStats}
            lightStats={lightStats}
            statsLoading={statsLoading}
            statsError={statsError}
            statsDays={statsDays}
            setStatsDays={setStatsDays}
            lightOnSeconds={lightOnSeconds}
          />
        )}

        {!status && !loading && (
          <Text style={styles.pullHint}>{T.pullToRetry}</Text>
        )}
      </ScrollView>

      <View style={[styles.tabBar, { paddingBottom: 8 + insets.bottom }]}>
        <Pressable style={[styles.tabItem, tab === 'home' && styles.tabItemActive]} onPress={() => setTab('home')}>
          <HomeIcon color={tab === 'home' ? '#111827' : '#4b5563'} />
          <Text style={[styles.tabLabel, tab === 'home' && styles.tabLabelActive]}>{T.tabHomeSubtitle}</Text>
        </Pressable>
        <Pressable style={[styles.tabItem, tab === 'info' && styles.tabItemActive]} onPress={() => setTab('info')}>
          <InfoIcon color={tab === 'info' ? '#111827' : '#4b5563'} />
          <Text style={[styles.tabLabel, tab === 'info' && styles.tabLabelActive]}>{T.tabInfo}</Text>
        </Pressable>
        <Pressable style={[styles.tabItem, tab === 'settings' && styles.tabItemActive]} onPress={() => setTab('settings')}>
          <SettingsIcon color={tab === 'settings' ? '#111827' : '#4b5563'} />
          <Text style={[styles.tabLabel, tab === 'settings' && styles.tabLabelActive]}>{T.tabSettings}</Text>
        </Pressable>
        <Pressable style={[styles.tabItem, tab === 'stats' && styles.tabItemActive]} onPress={() => setTab('stats')}>
          <StatsIcon color={tab === 'stats' ? '#111827' : '#4b5563'} />
          <Text style={[styles.tabLabel, tab === 'stats' && styles.tabLabelActive]}>{T.tabStats}</Text>
        </Pressable>
      </View>

      <FeedingCelebration visible={showFeedCelebration} />
    </AquariumScreen>
  )
}

// ─── Home tab ──────────────────────────────────────────────────────────────

function HomeTab({
  status,
  T,
  feeding,
  feedRepeats,
  setFeedRepeats,
  cooldown,
  feedError,
  isOnline,
  onFeed,
  localSchedule,
  scheduleDirty,
  onAddFeedTime,
  onRemoveFeedTime,
  onUpdateFeedTime,
  onApplyDays,
  onSaveSchedule,
  sAngle,
  onAngleChange,
  sSpeed,
  speedDirty,
  onSpeedChange,
  onSaveSpeed,
  sLightEnabled,
  lightOnSeconds,
  isLightOn,
  sFoodEnabled,
  foodTotalG,
  foodRemaining,
  foodPercent,
  foodBarColor,
  gramsPerFeed,
  feedTimesPerDay,
  foodMonths,
  foodDays,
}: {
  status: StatusResponse
  T: Translation
  feeding: boolean
  feedRepeats: number
  setFeedRepeats: (n: number) => void
  cooldown: number
  feedError: string | null
  isOnline: boolean | null
  onFeed: () => void
  localSchedule: LocalFeedTime[]
  scheduleDirty: boolean
  onAddFeedTime: () => void
  onRemoveFeedTime: (idx: number) => void
  onUpdateFeedTime: (idx: number, field: keyof LocalFeedTime, value: string | number) => void
  onApplyDays: (idx: number, days: string[]) => void
  onSaveSchedule: () => void
  sAngle: number
  onAngleChange: (v: number) => void
  sSpeed: number
  speedDirty: boolean
  onSpeedChange: (v: number) => void
  onSaveSpeed: () => void
  sLightEnabled: boolean
  lightOnSeconds: number
  isLightOn: boolean
  sFoodEnabled: boolean
  foodTotalG: number
  foodRemaining: number
  foodPercent: number
  foodBarColor: string
  gramsPerFeed: number
  feedTimesPerDay: number
  foodMonths: number
  foodDays: number
}) {
  const offline = isOnline === false
  return (
    <>
      <View style={styles.card}>
        <View style={styles.gaugesRow}>
          <BatteryGauge
            percent={status.batteryPercent}
            voltage={status.batteryVoltage}
            title={T.batteryStatus}
            voltageLabel={T.voltageValue}
          />
          <NextFeedGauge nextFeedMinutes={status.nextFeedMinutes} title={T.untilNextFeed} formatLabel={T.nextFeedIn} />
          {sLightEnabled && (
            <LightGauge
              durationSeconds={lightOnSeconds}
              isOn={isLightOn}
              title={T.lightSensor}
              onLabel={T.lightOn}
              offLabel={T.lightOff}
            />
          )}
        </View>

        {status.isCharging && (
          <View style={[styles.alert, styles.alertCharging]}>
            <Text>⚡ {T.isCharging}</Text>
          </View>
        )}
        {!status.isCharging && status.batteryPercent < 20 && (
          <View style={[styles.alert, styles.alertLow]}>
            <Text>! {T.lowBattery}</Text>
          </View>
        )}
      </View>

      {sFoodEnabled && foodTotalG > 0 && (
        <View style={styles.card}>
          <Text style={styles.foodTitle}>{T.foodTitle}</Text>
          <View style={styles.foodBarTrack}>
            <View style={[styles.foodBarFill, { width: `${foodPercent}%`, backgroundColor: foodBarColor }]} />
          </View>
          <View style={styles.foodMetaRow}>
            <Text style={styles.foodMetaText}>{T.foodRemaining(Math.round(foodRemaining))}</Text>
            <Text style={styles.foodMetaText}>
              {gramsPerFeed > 0 && feedTimesPerDay > 0 ? T.foodDuration(foodMonths, foodDays) : T.foodNoSchedule}
            </Text>
          </View>
        </View>
      )}

      <View style={styles.card}>
        <SectionHeader icon="clock" title={T.manualFeed} sub={T.manualFeedSub} />

        <View style={styles.repeatsRow}>
          <Text style={styles.repeatsLabel}>{T.repeatCount}</Text>
          <View style={styles.stepper}>
            <Pressable
              style={styles.stepperBtn}
              onPress={() => setFeedRepeats(Math.max(1, feedRepeats - 1))}
              disabled={feedRepeats <= 1}
            >
              <Text style={styles.stepperBtnText}>−</Text>
            </Pressable>
            <Text style={styles.stepperVal}>{feedRepeats}</Text>
            <Pressable
              style={styles.stepperBtn}
              onPress={() => setFeedRepeats(Math.min(10, feedRepeats + 1))}
              disabled={feedRepeats >= 10}
            >
              <Text style={styles.stepperBtnText}>+</Text>
            </Pressable>
          </View>
        </View>

        {feedError && <Text style={styles.error}>{feedError}</Text>}
        {offline && <Text style={styles.error}>{T.offline}</Text>}

        <Pressable
          style={[styles.feedButton, offline && styles.feedButtonDisabled]}
          onPress={onFeed}
          disabled={feeding || cooldown > 0 || offline}
        >
          {feeding ? (
            <ActivityIndicator color="#fff" />
          ) : cooldown > 0 ? (
            <View style={{ alignItems: 'center' }}>
              <Text style={styles.feedButtonCooldownTime}>
                {String(Math.floor(cooldown / 60)).padStart(2, '0')}:{String(cooldown % 60).padStart(2, '0')}
              </Text>
              <Text style={styles.feedButtonCooldownLabel}>{T.feedCooldownLabel}</Text>
            </View>
          ) : (
            <Text style={styles.feedButtonText}>{T.feedNow}</Text>
          )}
        </Pressable>
      </View>

      <View style={styles.card}>
        <SectionHeader icon="calendar" title={T.schedule} sub={T.scheduleSub} />

        {status.nextFeedMinutes != null && (
          <Text style={styles.infoBanner}>
            {T.nextFeedLabel(Math.floor(status.nextFeedMinutes / 60), status.nextFeedMinutes % 60)}
          </Text>
        )}

        {localSchedule.map((ft, idx) => (
          <View key={idx} style={styles.scheduleEditRow}>
            <View style={styles.scheduleTimeRow}>
              <NumberField
                label={T.hour}
                value={ft.hour}
                onChange={(n) => onUpdateFeedTime(idx, 'hour', n)}
                min={0}
                max={23}
                compact
              />
              <Text style={styles.scheduleColon}>:</Text>
              <NumberField
                label={T.minute}
                value={ft.minute}
                onChange={(n) => onUpdateFeedTime(idx, 'minute', n)}
                min={0}
                max={59}
                compact
              />
              <View style={styles.stepperCompact}>
                <Pressable
                  style={styles.stepperBtnSm}
                  onPress={() => onUpdateFeedTime(idx, 'repeats', Math.max(1, ft.repeats - 1))}
                  disabled={ft.repeats <= 1}
                >
                  <Text style={styles.stepperBtnText}>−</Text>
                </Pressable>
                <Text style={styles.stepperValSm}>×{ft.repeats}</Text>
                <Pressable
                  style={styles.stepperBtnSm}
                  onPress={() => onUpdateFeedTime(idx, 'repeats', Math.min(10, ft.repeats + 1))}
                  disabled={ft.repeats >= 10}
                >
                  <Text style={styles.stepperBtnText}>+</Text>
                </Pressable>
              </View>

              <Pressable style={styles.removeBtn} onPress={() => onRemoveFeedTime(idx)}>
                <Text style={styles.removeBtnText}>×</Text>
              </Pressable>
            </View>

            <DaySelect
              label={T.day}
              value={ft.day}
              days={T.days}
              onApply={(selected) => onApplyDays(idx, selected)}
            />
          </View>
        ))}

        <Pressable style={styles.addFeedBtn} onPress={onAddFeedTime}>
          <Text style={styles.addFeedBtnText}>{T.addFeeding}</Text>
        </Pressable>

        {scheduleDirty && <SaveButton label={T.saveAllTimes} onPress={onSaveSchedule} />}
      </View>

      <View style={styles.card}>
        <SectionHeader icon="servo" title={T.servoControl} sub={T.servoControlSub} />

        <Text style={styles.fieldLabel}>{T.servoAngle(sAngle)}</Text>
        <Slider
          style={styles.slider}
          minimumValue={0}
          maximumValue={180}
          step={1}
          value={sAngle}
          onValueChange={onAngleChange}
          minimumTrackTintColor="#2563eb"
          thumbTintColor="#2563eb"
        />

        <Text style={[styles.fieldLabel, { marginTop: 16 }]}>{T.servoSpeed(sSpeed)}</Text>
        <Slider
          style={styles.slider}
          minimumValue={10}
          maximumValue={20}
          step={0.5}
          value={sSpeed}
          onValueChange={onSpeedChange}
          minimumTrackTintColor="#2563eb"
          thumbTintColor="#2563eb"
        />

        {speedDirty && <SaveButton label={T.saveSpeed} onPress={onSaveSpeed} />}
      </View>
    </>
  )
}

// ─── Info tab ──────────────────────────────────────────────────────────────

function InfoTab({
  status,
  T,
  feedEvents,
  feedEventsLoading,
}: {
  status: StatusResponse
  T: Translation
  feedEvents: DeviceFeedEvent[]
  feedEventsLoading: boolean
}) {
  const uptimeSec = status.uptimeSeconds ?? 0
  const uptimeH = Math.floor(uptimeSec / 3600)
  const uptimeM = Math.floor((uptimeSec % 3600) / 60)

  return (
    <>
      <InfoCard title={T.wifiInfo} sub={T.wifiInfoSub} icon="wifi">
        <InfoRow label={T.ssid} value={status.wifiSSID || T.notConfigured} />
        <InfoRow label={T.ipAddr} value={status.wifiIP || T.notConnected} mono />
        <InfoRow label={T.modeLabel} value={status.isAPMode ? T.apMode : T.staMode} />
      </InfoCard>

      <InfoCard title={T.batterySection} sub={T.batterySectionSub} icon="battery">
        <InfoRow label={T.voltageValue} value={`${status.batteryVoltage.toFixed(2)} V`} />
        <InfoRow label={T.percentValue} value={`${status.batteryPercent}%`} />
        <InfoRow label={T.isCharging} value={status.isCharging ? T.on : T.off} />
      </InfoCard>

      <InfoCard title={T.manualControl} sub={T.manualControlSub} icon="gear">
        <InfoRow label={T.servoSpeedLabel} value={String(status.speed)} />
        <InfoRow label={T.feedRepeatsLabel} value={String(status.feedRepeats)} />
        <InfoRow label={T.scheduleCountLabel} value={String(status.feedTimes.length)} />
        <InfoRow label={T.powerSaveLabel} value={status.powerSaveMode ? T.on : T.off} />
        <InfoRow label={T.oledLabel} value={status.displayEnabled ? T.on : T.off} />
      </InfoCard>

      <InfoCard title={T.systemInfo} sub={T.systemInfoSub} icon="monitor">
        <InfoRow label={T.uptime} value={T.uptimeFmt(uptimeH, uptimeM)} />
        {status.currentTime && <InfoRow label={T.deviceTime} value={status.currentTime} mono />}
        {status.cpuFrequency != null && <InfoRow label={T.cpuFreq} value={`${status.cpuFrequency} MHz`} />}
      </InfoCard>

      {status.memoryTotalHeap != null && (
        <InfoCard title={T.memoryInfo} sub={T.memoryInfoSub} icon="grid">
          {status.memoryFreeHeap != null && <InfoRow label={T.freeMemory} value={toKB(status.memoryFreeHeap)} />}
          {status.memoryUsedHeap != null && <InfoRow label={T.usedMemory} value={toKB(status.memoryUsedHeap)} />}
          {status.memoryTotalHeap != null && <InfoRow label={T.totalMemory} value={toKB(status.memoryTotalHeap)} />}
          {status.memoryMaxAllocHeap != null && <InfoRow label={T.maxBlock} value={toKB(status.memoryMaxAllocHeap)} />}
          {status.memoryMinFreeHeap != null && <InfoRow label={T.minFree} value={toKB(status.memoryMinFreeHeap)} />}
          {status.cacheSize != null && <InfoRow label={T.cacheSizeLabel} value={toKB(status.cacheSize)} />}
          {status.cacheAge != null && <InfoRow label={T.cacheAgeLabel} value={`${status.cacheAge} ms`} />}
          {status.cacheValid != null && (
            <InfoRow label={T.cacheStatusLabel} value={status.cacheValid ? T.cacheActive : T.cacheInactive} />
          )}
        </InfoCard>
      )}

      <View style={styles.card}>
        <SectionHeader icon="document" title={T.feedJournal} sub={T.feedJournalSub} />
        {feedEventsLoading ? (
          <ActivityIndicator style={{ marginTop: 8 }} />
        ) : feedEvents.length === 0 ? (
          <Text style={styles.journalEmpty}>{T.feedJournalEmpty}</Text>
        ) : (
          feedEvents.map((ev) => (
            <View key={ev.id} style={styles.journalRow}>
              <Text style={styles.journalTime}>{new Date(ev.created_at).toLocaleString('uk-UA', { dateStyle: 'short', timeStyle: 'short' })}</Text>
              <Text style={styles.journalRepeats}>×{ev.repeats}</Text>
              <Text style={styles.journalSource}>{T.feedJournalSource(ev.source)}</Text>
            </View>
          ))
        )}
      </View>
    </>
  )
}

function InfoCard({
  title,
  sub,
  icon,
  children,
}: {
  title: string
  sub: string
  icon: SectionIconName
  children: React.ReactNode
}) {
  return (
    <View style={styles.card}>
      <SectionHeader icon={icon} title={title} sub={sub} />
      {children}
    </View>
  )
}

function SectionHeader({ icon, title, sub }: { icon: SectionIconName; title: string; sub: string }) {
  return (
    <View style={styles.sectionHeaderRow}>
      <SectionIcon name={icon} />
      <View style={styles.sectionHeaderText}>
        <Text style={styles.sectionTitle}>{title}</Text>
        <Text style={styles.sectionSub}>{sub}</Text>
      </View>
    </View>
  )
}

function InfoRow({ label, value, mono }: { label: string; value: string; mono?: boolean }) {
  return (
    <View style={styles.infoRow}>
      <Text style={styles.infoRowLabel}>{label}</Text>
      <Text style={[styles.infoRowValue, mono && styles.mono]}>{value}</Text>
    </View>
  )
}

// ─── Settings tab ──────────────────────────────────────────────────────────

function SettingsTab({
  status,
  T,
  lang,
  setLang,
  sPowerSave,
  sDeepSleep,
  setSDeepSleep,
  powerDirty,
  onPowerSaveToggle,
  onSavePowerSettings,
  sDisplayEnabled,
  onDisplayEnabledToggle,
  sDisplayOff,
  setSDisplayOff,
  sMinInterval,
  setSMinInterval,
  intervalDirty,
  onSaveInterval,
  sCalibVoltage,
  setSCalibVoltage,
  calibDirty,
  onApplyCalibration,
  sTimezone,
  setSTimezone,
  timezoneDirty,
  onSaveTimezone,
  otaFile,
  otaState,
  otaError,
  onPickOtaFile,
  onOtaUpload,
  forgetting,
  onForgetWifi,
  sLightEnabled,
  setSLightEnabled,
  sLightThreshold,
  setSLightThreshold,
  sFoodEnabled,
  setSFoodEnabled,
  foodInputG,
  setFoodInputG,
  foodInputGpf,
  setFoodInputGpf,
  onSaveFood,
}: {
  status: StatusResponse
  T: Translation
  lang: Lang
  setLang: (l: Lang) => void
  sPowerSave: boolean
  sDeepSleep: number
  setSDeepSleep: (n: number) => void
  powerDirty: boolean
  onPowerSaveToggle: (v: boolean) => void
  onSavePowerSettings: () => void
  sDisplayEnabled: boolean
  onDisplayEnabledToggle: (v: boolean) => void
  sDisplayOff: number
  setSDisplayOff: (n: number) => void
  sMinInterval: number
  setSMinInterval: (n: number) => void
  intervalDirty: boolean
  onSaveInterval: () => void
  sCalibVoltage: string
  setSCalibVoltage: (v: string) => void
  calibDirty: boolean
  onApplyCalibration: () => void
  sTimezone: number
  setSTimezone: (n: number) => void
  timezoneDirty: boolean
  onSaveTimezone: () => void
  otaFile: DocumentPicker.DocumentPickerAsset | null
  otaState: 'idle' | 'uploading' | 'rebooting' | 'done' | 'error'
  otaError: string
  onPickOtaFile: () => void
  onOtaUpload: () => void
  forgetting: boolean
  onForgetWifi: () => void
  sLightEnabled: boolean
  setSLightEnabled: (v: boolean) => void
  sLightThreshold: number
  setSLightThreshold: (v: number) => void
  sFoodEnabled: boolean
  setSFoodEnabled: (v: boolean) => void
  foodInputG: string
  setFoodInputG: (v: string) => void
  foodInputGpf: string
  setFoodInputGpf: (v: string) => void
  onSaveFood: () => void
}) {
  return (
    <>
      <View style={styles.card}>
        <SectionHeader icon="globe" title={T.langTitle} sub={T.langSub} />
        <View style={{ flexDirection: 'row', gap: 8 }}>
          <Pressable
            style={[styles.langChoiceBtn, lang === 'uk' && styles.langChoiceBtnActive]}
            onPress={() => setLang('uk')}
          >
            <Text style={[styles.langChoiceText, lang === 'uk' && styles.langChoiceTextActive]}>🇺🇦 Українська</Text>
          </Pressable>
          <Pressable
            style={[styles.langChoiceBtn, lang === 'en' && styles.langChoiceBtnActive]}
            onPress={() => setLang('en')}
          >
            <Text style={[styles.langChoiceText, lang === 'en' && styles.langChoiceTextActive]}>🇬🇧 English</Text>
          </Pressable>
        </View>
      </View>

      <View style={styles.card}>
        <View style={styles.settingsHeaderRow}>
          <View style={{ flex: 1 }}>
            <SectionHeader icon="power" title={T.powerSettings} sub={T.powerSettingsSub} />
          </View>
          <Switch value={sPowerSave} onValueChange={onPowerSaveToggle} />
        </View>
        {sPowerSave && (
          <NumberField
            label={T.deepSleepLabel}
            value={sDeepSleep}
            onChange={setSDeepSleep}
            min={10}
            max={3600}
          />
        )}
        {powerDirty && <SaveButton label={T.save} onPress={onSavePowerSettings} />}
      </View>

      <View style={styles.card}>
        <View style={styles.settingsHeaderRow}>
          <View style={{ flex: 1 }}>
            <SectionHeader icon="monitor" title={T.oledTitle} sub={T.oledSub} />
          </View>
          <Switch value={sDisplayEnabled} onValueChange={onDisplayEnabledToggle} />
        </View>
        <NumberField label={T.displayOffLabel(sDisplayOff)} value={sDisplayOff} onChange={setSDisplayOff} min={5} max={600} />
        {powerDirty && <SaveButton label={T.save} onPress={onSavePowerSettings} />}
      </View>

      <View style={styles.card}>
        <SectionHeader icon="clock" title={T.feedInterval} sub={T.feedIntervalSub} />
        <NumberField label={T.minIntervalLabel} value={sMinInterval} onChange={setSMinInterval} min={1} max={1440} />
        <Text style={styles.settingsHint}>{T.minIntervalHint(sMinInterval)}</Text>
        {intervalDirty && <SaveButton label={T.saveInterval} onPress={onSaveInterval} />}
      </View>

      <View style={styles.card}>
        <View style={styles.settingsHeaderRow}>
          <View style={{ flex: 1 }}>
            <SectionHeader icon="list" title={T.foodTitle} sub={T.foodSettingsSub} />
          </View>
          <Switch value={sFoodEnabled} onValueChange={setSFoodEnabled} />
        </View>
        {sFoodEnabled && (
          <>
            <NumberField label={T.foodGramsTotal} value={parseFloat(foodInputG) || 0} onChange={(n) => setFoodInputG(String(n))} min={0} max={100000} />
            <NumberField label={T.foodGramsPerFeed} value={parseFloat(foodInputGpf) || 0} onChange={(n) => setFoodInputGpf(String(n))} min={0} max={1000} />
            <SaveButton label={T.foodSave} onPress={onSaveFood} />
          </>
        )}
      </View>

      <View style={styles.card}>
        <View style={styles.settingsHeaderRow}>
          <View style={{ flex: 1 }}>
            <SectionHeader icon="sun" title={T.lightSensor} sub={T.lightSettingsSub} />
          </View>
          <Switch value={sLightEnabled} onValueChange={setSLightEnabled} />
        </View>
        {sLightEnabled && (
          <>
            <NumberField label={T.lightThresholdLabel} value={sLightThreshold} onChange={setSLightThreshold} min={1} max={1000} />
            <Text style={styles.settingsHint}>{T.lightThresholdHint}</Text>
          </>
        )}
      </View>

      <View style={styles.card}>
        <SectionHeader icon="calibration" title={T.calibration} sub={T.calibrationSub} />
        <Text style={styles.settingsHint}>{T.calibrationCurrent(status.batteryVoltage.toFixed(3))}</Text>
        <Text style={styles.fieldLabel}>{T.calibrationLabel}</Text>
        <TextInput
          style={styles.textField}
          value={sCalibVoltage}
          onChangeText={setSCalibVoltage}
          keyboardType="decimal-pad"
          placeholder="e.g. 3.742"
        />
        <Text style={styles.settingsHint}>{T.calibrationHint}</Text>
        {calibDirty && <SaveButton label={T.applyCalibration} onPress={onApplyCalibration} />}
      </View>

      <View style={styles.card}>
        <SectionHeader icon="globe" title={T.timezone} sub={T.timezoneSub} />
        <Text style={styles.fieldLabel}>{T.timezoneLabel}</Text>
        <View style={styles.stepper}>
          <Pressable style={styles.stepperBtn} onPress={() => setSTimezone(Math.max(-12, sTimezone - 1))}>
            <Text style={styles.stepperBtnText}>−</Text>
          </Pressable>
          <Text style={styles.stepperVal}>UTC{sTimezone >= 0 ? `+${sTimezone}` : sTimezone}</Text>
          <Pressable style={styles.stepperBtn} onPress={() => setSTimezone(Math.min(14, sTimezone + 1))}>
            <Text style={styles.stepperBtnText}>+</Text>
          </Pressable>
        </View>
        <Text style={styles.settingsHint}>{T.timezoneHint}</Text>
        {timezoneDirty && <SaveButton label={T.saveTimezone} onPress={onSaveTimezone} />}
      </View>

      <View style={styles.card}>
        <SectionHeader icon="cloudUpload" title={T.otaTitle} sub={`${T.otaCurrentVersion} ${status.firmwareVersion ?? '—'}`} />

        {status.batteryPercent < 20 && (
          <View style={[styles.alert, styles.alertLow]}>
            <Text style={{ fontSize: 12 }}>{T.otaBatteryWarning}</Text>
          </View>
        )}

        {(otaState === 'idle' || otaState === 'error') && (
          <>
            <Pressable style={styles.otaDropZone} onPress={onPickOtaFile}>
              <Text style={styles.otaDropZoneText}>{otaFile ? otaFile.name : T.otaSelectFile}</Text>
            </Pressable>
            {otaFile && (
              <SaveButton label={T.otaFlash} onPress={onOtaUpload} />
            )}
            {otaState === 'error' && <Text style={styles.error}>{otaError}</Text>}
          </>
        )}

        {otaState === 'uploading' && (
          <View style={styles.otaProgress}>
            <ActivityIndicator />
            <Text style={styles.settingsHint}>{T.otaUploading}</Text>
          </View>
        )}
        {otaState === 'rebooting' && (
          <View style={styles.otaProgress}>
            <ActivityIndicator />
            <Text style={styles.settingsHint}>{T.otaRebooting}</Text>
          </View>
        )}
        {otaState === 'done' && <Text style={styles.otaDone}>{T.otaDone}</Text>}
      </View>

      <View style={[styles.card, styles.dangerCard]}>
        <Text style={[styles.sectionTitle, styles.dangerTitle]}>{T.dangerZone}</Text>
        <Text style={styles.sectionSub}>{T.forgetWifiSub}</Text>
        <Pressable style={styles.dangerButton} onPress={onForgetWifi} disabled={forgetting}>
          {forgetting ? <ActivityIndicator color="#fff" /> : <Text style={styles.dangerButtonText}>{T.forgetWifi}</Text>}
        </Pressable>
      </View>
    </>
  )
}

function NumberField({
  label,
  value,
  onChange,
  min,
  max,
  compact,
}: {
  label: string
  value: number
  onChange: (n: number) => void
  min: number
  max: number
  compact?: boolean
}) {
  return (
    <View style={compact ? undefined : { marginTop: 8 }}>
      <Text style={styles.fieldLabel}>{label}</Text>
      <TextInput
        style={[styles.textField, compact && styles.textFieldCompact]}
        value={String(value)}
        onChangeText={(t) => {
          const n = parseInt(t, 10)
          if (!isNaN(n)) onChange(Math.max(min, Math.min(max, n)))
          else if (t === '') onChange(min)
        }}
        keyboardType="number-pad"
        maxLength={2}
      />
    </View>
  )
}

function SaveButton({ label, onPress }: { label: string; onPress: () => void }) {
  return (
    <Pressable style={styles.saveBtn} onPress={onPress}>
      <Text style={styles.saveBtnText}>{label}</Text>
    </Pressable>
  )
}

function DaySelect({
  label,
  value,
  days,
  onApply,
}: {
  label: string
  value: string
  days: readonly string[]
  onApply: (selectedDays: string[]) => void
}) {
  const [open, setOpen] = useState(false)
  const [selected, setSelected] = useState<string[]>([value])
  const idx = parseInt(value, 10)
  const current = days[idx] ?? days[0]
  const summary = selected.length > 1 ? selected.map((v) => days[parseInt(v, 10)]).join(', ') : current

  const openModal = () => {
    setSelected([value])
    setOpen(true)
  }

  const toggle = (v: string) => {
    setSelected((prev) => {
      // "Щодня" (index 0) is exclusive — picking it clears everything else, and vice versa.
      if (v === '0') return prev.includes('0') ? [] : ['0']
      const withoutEveryDay = prev.filter((d) => d !== '0')
      return withoutEveryDay.includes(v) ? withoutEveryDay.filter((d) => d !== v) : [...withoutEveryDay, v]
    })
  }

  const apply = () => {
    if (selected.length > 0) onApply(selected)
    setOpen(false)
  }

  return (
    <>
      <Pressable style={styles.daySelectBtn} onPress={openModal}>
        <Text style={styles.daySelectLabel}>{label}</Text>
        <Text style={styles.daySelectValue} numberOfLines={1}>
          {summary} ▾
        </Text>
      </Pressable>

      <Modal visible={open} transparent animationType="fade" onRequestClose={() => setOpen(false)}>
        <Pressable style={styles.modalOverlay} onPress={() => setOpen(false)}>
          <Pressable style={styles.modalCard} onPress={(e) => e.stopPropagation()}>
            {days.map((d, i) => {
              const v = String(i)
              const checked = selected.includes(v)
              return (
                <Pressable key={v} style={styles.modalItem} onPress={() => toggle(v)}>
                  <Text style={[styles.modalItemText, checked && styles.modalItemTextActive]}>{d}</Text>
                  <View style={[styles.checkbox, checked && styles.checkboxChecked]}>
                    {checked && <Text style={styles.checkboxMark}>✓</Text>}
                  </View>
                </Pressable>
              )
            })}
            <Pressable style={styles.modalApplyBtn} onPress={apply} disabled={selected.length === 0}>
              <Text style={styles.modalApplyBtnText}>OK</Text>
            </Pressable>
          </Pressable>
        </Pressable>
      </Modal>
    </>
  )
}

// ─── Stats tab ─────────────────────────────────────────────────────────────

function StatsTab({
  T,
  status,
  deviceStats,
  lightStats,
  statsLoading,
  statsError,
  statsDays,
  setStatsDays,
  lightOnSeconds,
}: {
  T: Translation
  status: StatusResponse | null
  deviceStats: DeviceStats | null
  lightStats: LightStats | null
  statsLoading: boolean
  statsError: string | null
  statsDays: 7 | 14 | 30
  setStatsDays: (d: 7 | 14 | 30) => void
  lightOnSeconds: number
}) {
  const today = new Date().toISOString().slice(0, 10)

  const backendTodaySec = lightStats?.days.find((d) => d.date === today)?.duration_sec ?? 0
  const todayLightSec = backendTodaySec + lightOnSeconds
  const todayLightH = Math.floor(todayLightSec / 3600)
  const todayLightM = Math.floor((todayLightSec % 3600) / 60)

  const lightSparkline = (lightStats?.days ?? []).map((d) => {
    const sec = d.date === today ? d.duration_sec + lightOnSeconds : d.duration_sec
    return Math.round(sec / 60)
  })

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
  const recs = Array.from(new Set([...condRecs, tip1, tip2])).slice(0, 5)

  return (
    <>
      {statsError && <Text style={styles.infoBanner}>{statsError}</Text>}
      {statsLoading && <ActivityIndicator style={{ marginVertical: 12 }} />}

      <View style={styles.statsGrid}>
        <View style={styles.statsCard}>
          <Text style={styles.statsValue}>{deviceStats?.feedings_today ?? '--'}</Text>
          <Text style={styles.statsLabel}>{T.statsFeedingsToday}</Text>
        </View>
        <View style={styles.statsCard}>
          <Text style={styles.statsValue}>{deviceStats?.avg_feedings_per_day ?? '--'}</Text>
          <Text style={styles.statsLabel}>{T.statsAvgPerDay}</Text>
        </View>
        <View style={styles.statsCard}>
          <Text style={styles.statsValue}>{todayLightH > 0 || todayLightM > 0 ? T.statsLightHours(todayLightH, todayLightM) : '--'}</Text>
          <Text style={styles.statsLabel}>{T.statsLightToday}</Text>
        </View>
      </View>

      <View style={styles.periodBar}>
        {([7, 14, 30] as const).map((d) => (
          <Pressable
            key={d}
            style={[styles.periodPill, statsDays === d && styles.periodPillActive]}
            onPress={() => setStatsDays(d)}
          >
            <Text style={[styles.periodPillText, statsDays === d && styles.periodPillTextActive]}>{d}д</Text>
          </Pressable>
        ))}
      </View>

      <View style={styles.card}>
        <Text style={styles.sectionTitle}>{T.statsFeedings}</Text>
        <Sparkline data={deviceStats?.feedings_sparkline ?? []} id="dev-feedings" color="#1976D2" />
        <View style={styles.statsDaysRow}>
          {feedingDayLabels.map((label, i) => (
            <Text key={i} style={styles.statsDayLabel}>
              {label}
            </Text>
          ))}
        </View>
      </View>

      {status?.lightLux != null && (
        <View style={styles.card}>
          <Text style={styles.sectionTitle}>{T.statsLight}</Text>
          <Sparkline data={lightSparkline} id="dev-light" color="#D97706" />
          <View style={styles.statsDaysRow}>
            {lightDayLabels.map((label, i) => (
              <Text key={i} style={styles.statsDayLabel}>
                {label}
              </Text>
            ))}
          </View>
        </View>
      )}

      <View style={styles.card}>
        <Text style={styles.sectionTitle}>{T.statsRecommendations}</Text>
        {recs.map((r, i) => (
          <Text key={i} style={styles.recItem}>
            • {r}
          </Text>
        ))}
      </View>
    </>
  )
}

const styles = StyleSheet.create({
  center: { flex: 1, alignItems: 'center', justifyContent: 'center' },
  centerText: { color: '#fff', fontSize: 15 },
  centerTitle: { fontSize: 18, fontWeight: '700', color: '#fff6e4' },
  headerBar: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 16,
    paddingTop: 12,
    paddingBottom: 8,
    backgroundColor: 'rgba(255,255,255,0.96)',
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(15,23,42,0.07)',
  },
  backButton: {
    width: 34,
    height: 34,
    borderRadius: 17,
    backgroundColor: '#eef2ff',
    alignItems: 'center',
    justifyContent: 'center',
  },
  backText: { fontSize: 18, fontWeight: '800', color: '#1f8fb8' },
  brand: { flexDirection: 'row', alignItems: 'center', gap: 6 },
  brandTextCol: { alignItems: 'center' },
  brandTitle: { fontSize: 15, fontWeight: '700', color: '#1f2937', textAlign: 'center' },
  brandSubtitle: { fontSize: 11, color: '#9ca3af', textAlign: 'center' },
  headerSpacer: { width: 34 },
  langToggle: { flexDirection: 'row', gap: 4 },
  langBtn: { paddingHorizontal: 10, paddingVertical: 4, borderRadius: 6, backgroundColor: '#f0f0f0' },
  langBtnActive: { backgroundColor: '#2563eb' },
  langBtnText: { fontSize: 12, fontWeight: '600', color: '#333' },
  langBtnTextActive: { color: '#fff' },
  offlineBanner: { backgroundColor: '#fee2e2', paddingVertical: 6, alignItems: 'center' },
  offlineBannerText: { color: '#991b1b', fontWeight: '600', fontSize: 12 },
  error: { color: '#ffb199', fontWeight: '600', paddingHorizontal: 16, marginTop: 4 },
  content: { padding: 16, gap: 12, paddingBottom: 24 },
  card: { borderWidth: 1, borderColor: '#eee', borderRadius: 12, padding: 16, marginBottom: 12, backgroundColor: '#fafafa' },
  gaugesRow: { flexDirection: 'row', justifyContent: 'center', alignItems: 'flex-start', gap: 8 },
  alert: { marginTop: 12, padding: 10, borderRadius: 8 },
  alertCharging: { backgroundColor: '#e0f2fe' },
  alertLow: { backgroundColor: '#fee2e2' },
  foodTitle: { fontSize: 12, fontWeight: '700', color: '#444', marginBottom: 6 },
  foodBarTrack: { height: 8, borderRadius: 4, backgroundColor: '#eee', overflow: 'hidden' },
  foodBarFill: { height: 8, borderRadius: 4 },
  foodMetaRow: { flexDirection: 'row', justifyContent: 'space-between', marginTop: 6 },
  foodMetaText: { fontSize: 11, color: '#666' },
  sectionTitle: { fontSize: 15, fontWeight: '600', color: '#222' },
  sectionSub: { fontSize: 12, color: '#9099A6', marginTop: 2 },
  sectionHeaderRow: { flexDirection: 'row', alignItems: 'flex-start', gap: 10, marginBottom: 12 },
  sectionHeaderText: { flex: 1, justifyContent: 'center' },
  repeatsRow: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginBottom: 12 },
  repeatsLabel: { fontSize: 13, color: '#444' },
  stepper: { flexDirection: 'row', alignItems: 'center', gap: 10 },
  stepperBtn: { width: 32, height: 32, borderRadius: 16, backgroundColor: '#e5e7eb', alignItems: 'center', justifyContent: 'center' },
  stepperBtnText: { fontSize: 18, fontWeight: '700' },
  stepperVal: { fontSize: 16, fontWeight: '700', minWidth: 20, textAlign: 'center' },
  feedButton: { backgroundColor: '#2563eb', borderRadius: 999, paddingVertical: 16, alignItems: 'center' },
  feedButtonDisabled: { backgroundColor: '#9ca3af' },
  feedButtonText: { color: '#fff', fontSize: 16, fontWeight: '700' },
  feedButtonCooldownTime: { color: '#fff', fontSize: 18, fontWeight: '700' },
  feedButtonCooldownLabel: { color: '#dbeafe', fontSize: 11 },
  infoBanner: { backgroundColor: '#eef2ff', padding: 8, borderRadius: 6, fontSize: 12, color: '#333', marginBottom: 10 },
  scheduleEditRow: { paddingVertical: 12, borderTopWidth: 1, borderTopColor: '#eee', gap: 8 },
  scheduleTimeRow: { flexDirection: 'row', alignItems: 'center', gap: 6, flexWrap: 'wrap' },
  scheduleColon: { fontSize: 18, fontWeight: '700', marginHorizontal: 2 },
  removeBtn: { marginLeft: 'auto', width: 28, height: 28, borderRadius: 14, backgroundColor: '#fee2e2', alignItems: 'center', justifyContent: 'center' },
  removeBtnText: { color: '#c0392b', fontSize: 16, fontWeight: '700', lineHeight: 18 },
  stepperCompact: { flexDirection: 'row', alignItems: 'center', gap: 6, marginLeft: 8 },
  stepperValSm: { fontSize: 13, fontWeight: '700', minWidth: 22, textAlign: 'center' },
  stepperBtnSm: { width: 26, height: 26, borderRadius: 13, backgroundColor: '#e5e7eb', alignItems: 'center', justifyContent: 'center' },
  daySelectBtn: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    backgroundColor: '#f5f5f5',
    borderRadius: 8,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  daySelectLabel: { fontSize: 12, color: '#666' },
  daySelectValue: { fontSize: 13, fontWeight: '700', color: '#111827', flexShrink: 1, marginLeft: 8 },
  modalOverlay: { flex: 1, backgroundColor: 'rgba(0,0,0,0.4)', justifyContent: 'center', padding: 32 },
  modalCard: { backgroundColor: '#fff', borderRadius: 14, paddingVertical: 6, overflow: 'hidden' },
  modalItem: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', paddingHorizontal: 18, paddingVertical: 14, borderBottomWidth: 1, borderBottomColor: '#f0f0f0' },
  modalItemText: { fontSize: 15, color: '#333' },
  modalItemTextActive: { color: '#2563eb', fontWeight: '700' },
  checkbox: { width: 22, height: 22, borderRadius: 6, borderWidth: 1.5, borderColor: '#ccc', alignItems: 'center', justifyContent: 'center' },
  checkboxChecked: { backgroundColor: '#2563eb', borderColor: '#2563eb' },
  checkboxMark: { color: '#fff', fontSize: 13, fontWeight: '700' },
  modalApplyBtn: { alignItems: 'center', paddingVertical: 14, marginTop: 4 },
  modalApplyBtnText: { color: '#2563eb', fontWeight: '700', fontSize: 15 },
  addFeedBtn: { alignItems: 'center', paddingVertical: 10, marginTop: 8 },
  addFeedBtnText: { color: '#2563eb', fontWeight: '600', fontSize: 13 },
  infoRow: { flexDirection: 'row', justifyContent: 'space-between', paddingVertical: 6, borderTopWidth: 1, borderTopColor: '#eee' },
  infoRowLabel: { color: '#666', fontSize: 13 },
  infoRowValue: { fontSize: 13, fontWeight: '600' },
  mono: { fontFamily: 'monospace' },
  journalEmpty: { color: '#999', fontSize: 13, marginTop: 8 },
  journalRow: { flexDirection: 'row', justifyContent: 'space-between', paddingVertical: 6, borderTopWidth: 1, borderTopColor: '#eee' },
  journalTime: { fontSize: 12, color: '#444', flex: 2 },
  journalRepeats: { fontSize: 12, color: '#444', flex: 1, textAlign: 'center' },
  journalSource: { fontSize: 12, color: '#444', flex: 1, textAlign: 'right' },
  statsGrid: { flexDirection: 'row', gap: 8, marginBottom: 12 },
  statsCard: { flex: 1, borderWidth: 1, borderColor: '#eee', borderRadius: 12, padding: 12, backgroundColor: '#fafafa', alignItems: 'center', gap: 4 },
  statsValue: { fontSize: 20, fontWeight: '700', color: '#111827' },
  statsLabel: { fontSize: 10, color: '#888', textAlign: 'center' },
  periodBar: { flexDirection: 'row', gap: 6, marginBottom: 12 },
  periodPill: { paddingHorizontal: 14, paddingVertical: 6, borderRadius: 999, backgroundColor: '#f0f0f0' },
  periodPillActive: { backgroundColor: '#2563eb' },
  periodPillText: { fontSize: 12, fontWeight: '600', color: '#555' },
  periodPillTextActive: { color: '#fff' },
  statsDaysRow: { flexDirection: 'row', justifyContent: 'space-between', marginTop: 4 },
  statsDayLabel: { fontSize: 9, color: '#999' },
  recItem: { fontSize: 13, color: '#333', marginTop: 6, lineHeight: 18 },
  tabBar: {
    flexDirection: 'row',
    backgroundColor: 'rgba(255,255,255,0.96)',
    borderTopWidth: 1,
    borderTopColor: 'rgba(15,23,42,0.07)',
    borderTopLeftRadius: 16,
    borderTopRightRadius: 16,
    paddingTop: 8,
    paddingHorizontal: 8,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: -6 },
    shadowOpacity: 0.08,
    shadowRadius: 18,
    elevation: 8,
  },
  tabItem: { flex: 1, alignItems: 'center', gap: 4, paddingVertical: 8, marginHorizontal: 4, borderRadius: 12 },
  tabItemActive: { backgroundColor: 'rgba(15,23,42,0.08)' },
  tabLabel: { fontSize: 11, color: '#4b5563', fontWeight: '500' },
  tabLabelActive: { color: '#111827', fontWeight: '700' },
  pullHint: { textAlign: 'center', color: 'rgba(255,255,255,0.7)', marginTop: 40, fontSize: 13 },
  toast: { backgroundColor: '#111', paddingVertical: 8, alignItems: 'center', marginHorizontal: 16, borderRadius: 8, marginTop: 4 },
  toastText: { color: '#fff', fontSize: 12, fontWeight: '600' },
  langChoiceBtn: { flex: 1, borderWidth: 1, borderColor: '#ddd', borderRadius: 8, paddingVertical: 12, alignItems: 'center' },
  langChoiceBtnActive: { backgroundColor: '#2563eb', borderColor: '#2563eb' },
  langChoiceText: { fontSize: 14, color: '#333' },
  langChoiceTextActive: { color: '#fff', fontWeight: '600' },
  settingsHeaderRow: { flexDirection: 'row', alignItems: 'center', marginBottom: 4 },
  settingsHint: { fontSize: 11, color: '#888', marginTop: 6 },
  fieldLabel: { fontSize: 13, color: '#444', marginTop: 8, marginBottom: 4 },
  slider: { width: '100%', height: 36 },
  textField: {
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 8,
    paddingHorizontal: 12,
    paddingVertical: 8,
    fontSize: 15,
  },
  textFieldCompact: { width: 52, textAlign: 'center', paddingHorizontal: 4 },
  saveBtn: { backgroundColor: '#2563eb', borderRadius: 8, paddingVertical: 10, alignItems: 'center', marginTop: 12 },
  saveBtnText: { color: '#fff', fontWeight: '600', fontSize: 13 },
  otaDropZone: { borderWidth: 1, borderStyle: 'dashed', borderColor: '#ccc', borderRadius: 10, padding: 20, alignItems: 'center', marginTop: 8 },
  otaDropZoneText: { color: '#666', fontSize: 13, textAlign: 'center' },
  otaProgress: { alignItems: 'center', gap: 8, marginTop: 12 },
  otaDone: { color: '#16a34a', fontWeight: '600', marginTop: 12, textAlign: 'center' },
  dangerCard: { borderColor: '#fecaca', backgroundColor: '#fef2f2' },
  dangerTitle: { color: '#991b1b' },
  dangerButton: { backgroundColor: '#dc2626', borderRadius: 8, paddingVertical: 12, alignItems: 'center', marginTop: 12 },
  dangerButtonText: { color: '#fff', fontWeight: '600' },
})
