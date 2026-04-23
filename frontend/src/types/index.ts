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

