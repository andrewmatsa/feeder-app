import axios, { type InternalAxiosRequestConfig } from 'axios'
import type {
  AdminDevice,
  AdminFeedEvent,
  AdminStats,
  AdminUser,
  AngleRequest,
  AuthResponse,
  CalibrateRequest,
  CreateDeviceRequest,
  CreateLightEventRequest,
  CreateUserRequest,
  Device,
  DeviceFeedEvent,
  DeviceStats,
  DisplaySettingsRequest,
  FeedRequest,
  LightStats,
  LoginRequest,
  MinIntervalRequest,
  PowerModeRequest,
  RegisterRequest,
  ScheduleRequest,
  SpeedRequest,
  StatusResponse,
  TimezoneRequest,
  UpdateDeviceRequest,
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

  async getStatus(deviceId?: string): Promise<StatusResponse> {
    const response = await apiClient.get<StatusResponse>('/api/status', {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
    return response.data
  },

  async feedNow(request: FeedRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/feed', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setSpeed(request: SpeedRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/speed', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setSchedule(request: ScheduleRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/schedule', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setAngle(request: AngleRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/angle', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setPowerMode(request: PowerModeRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/power-mode', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setDisplaySettings(request: DisplaySettingsRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/display-settings', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setMinInterval(request: MinIntervalRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/min-interval', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async calibrateBattery(request: CalibrateRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/calibrate', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async setTimezone(request: TimezoneRequest, deviceId?: string): Promise<void> {
    await apiClient.post('/api/timezone', request, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async otaUpdate(file: File, deviceId?: string): Promise<void> {
    const formData = new FormData()
    formData.append('file', file)
    await apiClient.post('/api/ota-update', formData, {
      params: deviceId ? { device_id: deviceId } : undefined,
      timeout: 120_000,
    })
  },

  async forgetWifi(deviceId?: string): Promise<void> {
    await apiClient.post('/api/forget-wifi', null, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async listAdminUsers(): Promise<AdminUser[]> {
    const response = await apiClient.get<AdminUser[]>('/api/v1/admin/users')
    return response.data
  },

  async createAdminUser(request: CreateUserRequest): Promise<AdminUser> {
    const response = await apiClient.post<AdminUser>('/api/v1/admin/users', request)
    return response.data
  },

  async updateUserRole(userId: string, role: 'admin' | 'user'): Promise<AdminUser> {
    const response = await apiClient.patch<AdminUser>(`/api/v1/admin/users/${userId}/role`, { role })
    return response.data
  },

  async deleteAdminUser(userId: string): Promise<void> {
    await apiClient.delete(`/api/v1/admin/users/${userId}`)
  },

  async listUserDevices(userId: string): Promise<AdminDevice[]> {
    const response = await apiClient.get<AdminDevice[]>(`/api/v1/admin/users/${userId}/devices`)
    return response.data
  },

  async deleteUserDevice(userId: string, deviceId: string): Promise<void> {
    await apiClient.delete(`/api/v1/admin/users/${userId}/devices/${deviceId}`)
  },

  async getSimulatedOfflineDevices(): Promise<string[]> {
    const response = await apiClient.get<string[]>('/api/v1/admin/devices/sim-offline')
    return response.data
  },

  async setSimulatedOffline(deviceId: string): Promise<void> {
    await apiClient.post(`/api/v1/admin/devices/${deviceId}/sim-offline`)
  },

  async clearSimulatedOffline(deviceId: string): Promise<void> {
    await apiClient.delete(`/api/v1/admin/devices/${deviceId}/sim-offline`)
  },

  async getSimulatedLowBatteryDevices(): Promise<string[]> {
    const response = await apiClient.get<string[]>('/api/v1/admin/devices/sim-battery')
    return response.data
  },

  async setSimulatedLowBattery(deviceId: string): Promise<void> {
    await apiClient.post(`/api/v1/admin/devices/${deviceId}/sim-battery`)
  },

  async clearSimulatedLowBattery(deviceId: string): Promise<void> {
    await apiClient.delete(`/api/v1/admin/devices/${deviceId}/sim-battery`)
  },

  async getAdminStats(): Promise<AdminStats> {
    const response = await apiClient.get<AdminStats>('/api/v1/admin/stats')
    return response.data
  },

  async listAdminFeedEvents(limit = 50, offset = 0): Promise<AdminFeedEvent[]> {
    const response = await apiClient.get<AdminFeedEvent[]>('/api/v1/admin/feed-events', {
      params: { limit, offset },
    })
    return response.data
  },

  async getDeviceFeedEvents(deviceId: string, limit = 20): Promise<DeviceFeedEvent[]> {
    const response = await apiClient.get<DeviceFeedEvent[]>(
      `/api/v1/devices/${deviceId}/feed-events`,
      { params: { limit } },
    )
    return response.data
  },

  async getDeviceStats(deviceId: string, days = 7): Promise<DeviceStats> {
    const response = await apiClient.get<DeviceStats>(`/api/v1/devices/${deviceId}/stats`, {
      params: { days },
    })
    return response.data
  },

  async createLightEvent(deviceId: string, request: CreateLightEventRequest): Promise<void> {
    await apiClient.post(`/api/v1/devices/${deviceId}/light-events`, request)
  },

  async getLightStats(deviceId: string, days = 14): Promise<LightStats> {
    const response = await apiClient.get<LightStats>(`/api/v1/devices/${deviceId}/light-stats`, {
      params: { days },
    })
    return response.data
  },
}
