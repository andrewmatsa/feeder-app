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
  Device,
  CreateDeviceRequest,
  UpdateDeviceRequest,
  AuthResponse,
  LoginRequest,
  RegisterRequest,
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

// Docker dev: leave VITE_API_URL unset — browser calls same origin, Vite proxies /api and /auth.
const API_BASE_URL = import.meta.env.VITE_API_URL ?? ''

const apiClient = axios.create({
  baseURL: API_BASE_URL,
  headers: { 'Content-Type': 'application/json' },
  timeout: 10000,
})

export function setAccessToken(token: string | null) {
  if (token) {
    apiClient.defaults.headers.common.Authorization = `Bearer ${token}`
  } else {
    delete apiClient.defaults.headers.common.Authorization
  }
}

apiClient.interceptors.request.use((config) => {
  const token = authStorage.getToken()
  if (token) config.headers.Authorization = `Bearer ${token}`
  return config
})

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
  (response) => response,
  async (error) => {
    const originalRequest = error.config as RetryableRequest
    if (error.response?.status !== 401 || originalRequest._retry) {
      return Promise.reject(error)
    }

    originalRequest._retry = true

    try {
      if (!refreshPromise) {
        refreshPromise = refreshAccessToken().finally(() => {
          refreshPromise = null
        })
      }
      const newToken = await refreshPromise
      originalRequest.headers.Authorization = `Bearer ${newToken}`
      return apiClient(originalRequest)
    } catch {
      authStorage.clear()
      window.location.href = '/login'
      return Promise.reject(error)
    }
  },
)

export function getApiErrorMessage(err: unknown, fallback: string): string {
  if (axios.isAxiosError(err)) {
    const detail = err.response?.data?.detail
    if (typeof detail === 'string') return detail
    if (Array.isArray(detail)) return detail.map((d) => d.msg ?? String(d)).join(', ')
  }
  if (err instanceof Error) return err.message
  return fallback
}

export const api = {
  async login(request: LoginRequest): Promise<AuthResponse> {
    const response = await apiClient.post<AuthResponse>('/auth/login', request)
    authStorage.save(response.data.access_token, response.data.refresh_token)
    setAccessToken(response.data.access_token)
    return response.data
  },

  async register(request: RegisterRequest): Promise<AuthResponse> {
    const response = await apiClient.post<AuthResponse>('/auth/register', request)
    authStorage.save(response.data.access_token, response.data.refresh_token)
    setAccessToken(response.data.access_token)
    return response.data
  },

  async logout(): Promise<void> {
    try {
      await apiClient.post('/auth/logout')
    } catch {
      // ignore
    } finally {
      authStorage.clear()
      setAccessToken(null)
    }
  },

  async listDevices(): Promise<Device[]> {
    const response = await apiClient.get<Device[]>('/api/v1/devices')
    return response.data
  },

  async createDevice(request: CreateDeviceRequest = {}): Promise<Device> {
    const response = await apiClient.post<Device>('/api/v1/devices', request)
    return response.data
  },

  async getDevice(deviceId: string): Promise<Device> {
    const response = await apiClient.get<Device>(`/api/v1/devices/${deviceId}`)
    return response.data
  },

  async updateDevice(deviceId: string, request: UpdateDeviceRequest): Promise<Device> {
    const response = await apiClient.patch<Device>(`/api/v1/devices/${deviceId}`, request)
    return response.data
  },

  async deleteDevice(deviceId: string): Promise<void> {
    await apiClient.delete(`/api/v1/devices/${deviceId}`)
  },

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
