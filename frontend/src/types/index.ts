export interface FeedTime {
  hour: number
  minute: number
  repeats: number
}

export interface StatusResponse {
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
}

export interface LoginRequest {
  email: string
  password: string
}

export interface RegisterRequest {
  email: string
  password: string
}
