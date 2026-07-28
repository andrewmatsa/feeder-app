export interface FeedTime {
  hour: number
  minute: number
  repeats: number
  day?: number // -1=every day, 0=Sun, 1=Mon…6=Sat; absent treated as -1
}

export interface StatusResponse {
  firmwareVersion?: string
  angle: number
  speed: number
  feedRepeats: number
  minFeedIntervalMin: number
  powerSaveMode: boolean
  displayEnabled: boolean
  displayOffAfterSec: number
  deepSleepIdleSec: number
  batteryVoltage: number
  batteryPercent: number
  isCharging: boolean
  feedTimes: FeedTime[]
  nextFeedMinutes?: number | null
  nextFeedHour?: number | null
  nextFeedMinute?: number | null
  currentTime?: string | null
  manualFeedCooldownSeconds: number
  wifiSSID: string
  wifiIP: string
  isAPMode: boolean
  sleepReason: string
  sleepCountdownSeconds: number
  displayAwake: boolean
  timestamp: string
  cpuFrequency?: number | null
  memoryFreeHeap?: number | null
  memoryUsedHeap?: number | null
  memoryTotalHeap?: number | null
  memoryMaxAllocHeap?: number | null
  memoryMinFreeHeap?: number | null
  cacheSize?: number | null
  cacheAge?: number | null
  cacheValid?: boolean | null
  uptimeSeconds?: number | null
  lightLux?: number | null
  buildDate?: string | null
  buildTime?: string | null
}

export interface FeedRequest {
  repeats: number
}

export interface SpeedRequest {
  speed: number
}

export interface ScheduleRequest {
  times: FeedTime[]
}

export interface AngleRequest {
  angle: number
}

export interface PowerModeRequest {
  enabled: boolean
}

export interface DisplaySettingsRequest {
  powerSaveMode: boolean
  deepSleepIdleSec: number
  displayEnabled: boolean
  displayOffAfterSec: number
}

export interface MinIntervalRequest {
  minFeedIntervalMin: number
}

export interface CalibrateRequest {
  actualVoltage: number
}

export interface TimezoneRequest {
  offsetHours: number
}

export interface Device {
  id: string
  name: string
  macAddress?: string | null
  createdAt: string
  lastSeen?: string | null
}

export interface CreateDeviceRequest {
  name?: string
  macAddress?: string
}

export interface UpdateDeviceRequest {
  name?: string
}

export interface AuthResponse {
  access_token: string
  refresh_token: string
  user_id: string
  email: string
  role: string
  created_at?: string | null
}

export interface DeviceFeedEvent {
  id: string
  created_at: string
  source: string
  repeats: number
}

export interface DayFeedStat {
  date: string
  count: number
  total_repeats: number
}

export interface DeviceStats {
  feedings_today: number
  total_repeats_today: number
  avg_feedings_per_day: number
  feedings_sparkline: number[]
  repeats_sparkline: number[]
  days: DayFeedStat[]
}

export interface CreateLightEventRequest {
  started_at: string
  ended_at: string
  duration_sec: number
}

export interface LightDayStat {
  date: string
  duration_sec: number
  duration_min: number
}

export interface LightStats {
  days: LightDayStat[]
}

export interface LoginRequest {
  email: string
  password: string
}

export interface RegisterRequest {
  email: string
  password: string
}
