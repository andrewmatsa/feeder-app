import axios, { type InternalAxiosRequestConfig } from 'axios'
import type {
  StatusResponse,
  FeedRequest,
  SpeedRequest,
  ScheduleRequest,
  AngleRequest,
  PowerModeRequest,
  DisplaySettingsRequest,
  MinIntervalRequest,
  CalibrateRequest,
  TimezoneRequest,
} from '../types'

const TOKEN_KEY = 'aquafeed_token'
const REFRESH_KEY = 'aquafeed_refresh'

export const authStorage = {
  getToken: () => localStorage.getItem(TOKEN_KEY),
  getRefresh: () => localStorage.getItem(REFRESH_KEY),
  save: (token: string, refresh: string) => {
    localStorage.setItem(TOKEN_KEY, token)
    localStorage.setItem(REFRESH_KEY, refresh)
  },
  clear: () => {
    localStorage.removeItem(TOKEN_KEY)
    localStorage.removeItem(REFRESH_KEY)
  },
}

const apiClient = axios.create({
  baseURL: import.meta.env.VITE_API_URL || '',
  headers: { 'Content-Type': 'application/json' },
  timeout: 10000,
})

apiClient.interceptors.request.use(config => {
  const token = authStorage.getToken()
  if (token) config.headers.Authorization = `Bearer ${token}`
  return config
})

// Token refresh state — prevents concurrent refresh races
let refreshPromise: Promise<string> | null = null

async function refreshAccessToken(): Promise<string> {
  const refreshToken = authStorage.getRefresh()
  if (!refreshToken) throw new Error('No refresh token')

  const res = await fetch('/auth/refresh', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ refresh_token: refreshToken }),
  })
  if (!res.ok) throw new Error('Refresh failed')

  const data = await res.json()
  authStorage.save(data.access_token, data.refresh_token)
  return data.access_token
}

interface RetryableRequest extends InternalAxiosRequestConfig {
  _retry?: boolean
}

apiClient.interceptors.response.use(
  response => response,
  async (error) => {
    const originalRequest = error.config as RetryableRequest
    if (error.response?.status !== 401 || originalRequest._retry) {
      return Promise.reject(error)
    }

    originalRequest._retry = true

    try {
      // If a refresh is already in flight, wait for it instead of firing again
      if (!refreshPromise) {
        refreshPromise = refreshAccessToken().finally(() => { refreshPromise = null })
      }
      const newToken = await refreshPromise
      originalRequest.headers.Authorization = `Bearer ${newToken}`
      return apiClient(originalRequest)
    } catch {
      authStorage.clear()
      window.location.reload()
      return Promise.reject(error)
    }
  }
)

export const api = {
  async getStatus(): Promise<StatusResponse> {
    const response = await apiClient.get<StatusResponse>('/api/status')
    return response.data
  },

  async feedNow(request: FeedRequest): Promise<void> {
    await apiClient.post('/api/feed', request)
  },

  async setSpeed(request: SpeedRequest): Promise<void> {
    await apiClient.post('/api/speed', request)
  },

  async setSchedule(request: ScheduleRequest): Promise<void> {
    await apiClient.post('/api/schedule', request)
  },

  async setAngle(request: AngleRequest): Promise<void> {
    await apiClient.post('/api/angle', request)
  },

  async setPowerMode(request: PowerModeRequest): Promise<void> {
    await apiClient.post('/api/power-mode', request)
  },

  async setDisplaySettings(request: DisplaySettingsRequest): Promise<void> {
    await apiClient.post('/api/display-settings', request)
  },

  async setMinInterval(request: MinIntervalRequest): Promise<void> {
    await apiClient.post('/api/min-interval', request)
  },

  async calibrateBattery(request: CalibrateRequest): Promise<void> {
    await apiClient.post('/api/calibrate', request)
  },

  async setTimezone(request: TimezoneRequest): Promise<void> {
    await apiClient.post('/api/timezone', request)
  },
}
