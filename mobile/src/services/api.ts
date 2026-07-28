import axios, { type InternalAxiosRequestConfig } from 'axios'
import * as SecureStore from 'expo-secure-store'
import type {
  AngleRequest,
  AuthResponse,
  CalibrateRequest,
  CreateDeviceRequest,
  CreateLightEventRequest,
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
  getToken: () => SecureStore.getItemAsync(TOKEN_KEY),
  getRefresh: () => SecureStore.getItemAsync(REFRESH_KEY),
  save: async (token: string, refresh: string) => {
    await SecureStore.setItemAsync(TOKEN_KEY, token)
    await SecureStore.setItemAsync(REFRESH_KEY, refresh)
  },
  clear: async () => {
    await SecureStore.deleteItemAsync(TOKEN_KEY)
    await SecureStore.deleteItemAsync(REFRESH_KEY)
  },
}

// Same role as VITE_API_URL for the web SPA (see CLAUDE.md): the backend base URL.
// Set EXPO_PUBLIC_API_URL in mobile/.env to your machine's LAN IP (e.g. http://192.168.1.50:8000)
// since "localhost" on a device/simulator does not point at your dev machine.
const API_BASE_URL = process.env.EXPO_PUBLIC_API_URL ?? ''

const apiClient = axios.create({
  baseURL: API_BASE_URL,
  headers: { 'Content-Type': 'application/json' },
  timeout: 10000,
})

// Cheap, unauthenticated reachability check. Used after WiFi provisioning,
// where the phone may still be mid-handoff between the device's AP and the
// home network — polling this avoids firing a mutating request (createDevice)
// into a dead network and surfacing a raw timeout to the user.
export async function checkBackendReachable(): Promise<boolean> {
  const controller = new AbortController()
  const timer = setTimeout(() => controller.abort(), 2000)
  try {
    const res = await fetch(`${API_BASE_URL}/health`, { signal: controller.signal })
    return res.ok
  } catch {
    return false
  } finally {
    clearTimeout(timer)
  }
}

// Set by authStore so the interceptor below can flip isAuthenticated when a
// session is truly dead (refresh also failed) — without this, the store
// never learns the token was cleared, so guards like <Redirect href="/login">
// never fire and screens are left showing a raw "Not authenticated" error
// with no way forward except manually backing out and back in.
let onSessionExpired: (() => void) | null = null
export function setSessionExpiredHandler(handler: () => void) {
  onSessionExpired = handler
}

export function setAccessToken(token: string | null) {
  if (token) {
    apiClient.defaults.headers.common.Authorization = `Bearer ${token}`
  } else {
    delete apiClient.defaults.headers.common.Authorization
  }
}

apiClient.interceptors.request.use(async (config) => {
  const token = await authStorage.getToken()
  if (token) config.headers.Authorization = `Bearer ${token}`
  return config
})

let refreshPromise: Promise<string> | null = null

async function refreshAccessToken(): Promise<string> {
  const refreshToken = await authStorage.getRefresh()
  if (!refreshToken) throw new Error('No refresh token')

  const res = await fetch(`${API_BASE_URL}/auth/refresh`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ refresh_token: refreshToken }),
  })
  if (!res.ok) throw new Error('Refresh failed')

  const data = await res.json()
  await authStorage.save(data.access_token, data.refresh_token)
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
      await authStorage.clear()
      setAccessToken(null)
      onSessionExpired?.()
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
    await authStorage.save(response.data.access_token, response.data.refresh_token)
    setAccessToken(response.data.access_token)
    return response.data
  },

  async register(request: RegisterRequest): Promise<AuthResponse> {
    const response = await apiClient.post<AuthResponse>('/auth/register', request)
    await authStorage.save(response.data.access_token, response.data.refresh_token)
    setAccessToken(response.data.access_token)
    return response.data
  },

  async logout(): Promise<void> {
    try {
      await apiClient.post('/auth/logout')
    } catch {
      // ignore
    } finally {
      await authStorage.clear()
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

  async registerPushToken(deviceId: string, token: string): Promise<void> {
    await apiClient.post(`/api/v1/devices/${deviceId}/push-token`, { token })
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

  async forgetWifi(deviceId?: string): Promise<void> {
    await apiClient.post('/api/forget-wifi', null, {
      params: deviceId ? { device_id: deviceId } : undefined,
    })
  },

  async otaUpdate(file: { uri: string; name: string; mimeType?: string | null }, deviceId?: string): Promise<void> {
    const formData = new FormData()
    // React Native's FormData expects this {uri,name,type} shape instead of a browser File/Blob.
    formData.append('file', {
      uri: file.uri,
      name: file.name,
      type: file.mimeType || 'application/octet-stream',
    } as unknown as Blob)
    await apiClient.post('/api/ota-update', formData, {
      params: deviceId ? { device_id: deviceId } : undefined,
      headers: { 'Content-Type': 'multipart/form-data' },
      timeout: 120_000,
    })
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
