import axios from 'axios'
import type {
  StatusResponse,
  FeedRequest,
  SpeedRequest,
  ScheduleRequest,
  AngleRequest,
  PowerModeRequest,
} from '../types'

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:8000'

const apiClient = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
  timeout: 10000,
})

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
}

