import { create } from 'zustand'
import { api, authStorage, getApiErrorMessage, setAccessToken } from '../services/api'
import type { LoginRequest, RegisterRequest } from '../types'

interface AuthState {
  email: string | null
  isAuthenticated: boolean
  login: (request: LoginRequest) => Promise<void>
  register: (request: RegisterRequest) => Promise<void>
  logout: () => Promise<void>
  hydrate: () => void
}

export const useAuthStore = create<AuthState>((set) => ({
  email: null,
  isAuthenticated: !!authStorage.getToken(),

  hydrate: () => {
    setAccessToken(authStorage.getToken())
    set({ isAuthenticated: !!authStorage.getToken() })
  },

  login: async (request) => {
    try {
      const data = await api.login(request)
      set({ email: data.email, isAuthenticated: true })
    } catch (err) {
      throw new Error(getApiErrorMessage(err, 'Помилка входу'))
    }
  },

  register: async (request) => {
    try {
      const data = await api.register(request)
      set({ email: data.email, isAuthenticated: true })
    } catch (err) {
      throw new Error(getApiErrorMessage(err, 'Помилка реєстрації'))
    }
  },

  logout: async () => {
    await api.logout()
    set({ email: null, isAuthenticated: false })
  },
}))
