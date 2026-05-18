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

