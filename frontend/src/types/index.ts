export interface StatusResponse {
  angle: number
  speed: number
  feedRepeats: number
  powerSaveMode: boolean
  batteryVoltage: number
  batteryPercent: number
  feedTimes: string[]
  timestamp?: string
}

export interface FeedRequest {
  repeats: number
}

export interface SpeedRequest {
  speed: number
}

export interface ScheduleRequest {
  times: string[]
}

export interface AngleRequest {
  angle: number
}

export interface PowerModeRequest {
  enabled: boolean
}

