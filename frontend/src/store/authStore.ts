import { create } from 'zustand'
import { api, authStorage, getApiErrorMessage, setAccessToken } from '../services/api'
import type { LoginRequest, RegisterRequest } from '../types'

const ROLE_KEY = 'aquafeed_role'
const EMAIL_KEY = 'aquafeed_email'
const CREATED_AT_KEY = 'aquafeed_created_at'

const roleStorage = {
  get: () => localStorage.getItem(ROLE_KEY) as 'admin' | 'user' | null,
  save: (role: string) => localStorage.setItem(ROLE_KEY, role),
  clear: () => localStorage.removeItem(ROLE_KEY),
}

interface AuthState {
  email: string | null
  role: 'admin' | 'user' | null
  createdAt: string | null
  isAuthenticated: boolean
  isAdmin: boolean
  login: (request: LoginRequest) => Promise<void>
  register: (request: RegisterRequest) => Promise<void>
  logout: () => Promise<void>
  hydrate: () => void
}

export const useAuthStore = create<AuthState>((set) => ({
  email: localStorage.getItem(EMAIL_KEY),
  role: roleStorage.get(),
  createdAt: localStorage.getItem(CREATED_AT_KEY),
  isAuthenticated: !!authStorage.getToken(),
  isAdmin: roleStorage.get() === 'admin',

  hydrate: () => {
    setAccessToken(authStorage.getToken())
    const role = roleStorage.get()
    set({
      isAuthenticated: !!authStorage.getToken(),
      role,
      isAdmin: role === 'admin',
      email: localStorage.getItem(EMAIL_KEY),
      createdAt: localStorage.getItem(CREATED_AT_KEY),
    })
  },

  login: async (request) => {
    try {
      const data = await api.login(request)
      roleStorage.save(data.role)
      if (data.email) localStorage.setItem(EMAIL_KEY, data.email)
      if (data.created_at) localStorage.setItem(CREATED_AT_KEY, data.created_at)
      set({
        email: data.email,
        role: data.role as 'admin' | 'user',
        createdAt: data.created_at ?? null,
        isAuthenticated: true,
        isAdmin: data.role === 'admin',
      })
    } catch (err) {
      throw new Error(getApiErrorMessage(err, 'Помилка входу'))
    }
  },

  register: async (request) => {
    try {
      const data = await api.register(request)
      roleStorage.save(data.role)
      if (data.email) localStorage.setItem(EMAIL_KEY, data.email)
      if (data.created_at) localStorage.setItem(CREATED_AT_KEY, data.created_at)
      set({
        email: data.email,
        role: data.role as 'admin' | 'user',
        createdAt: data.created_at ?? null,
        isAuthenticated: true,
        isAdmin: data.role === 'admin',
      })
    } catch (err) {
      throw new Error(getApiErrorMessage(err, 'Помилка реєстрації'))
    }
  },

  logout: async () => {
    await api.logout()
    roleStorage.clear()
    localStorage.removeItem(EMAIL_KEY)
    localStorage.removeItem(CREATED_AT_KEY)
    set({ email: null, role: null, createdAt: null, isAuthenticated: false, isAdmin: false })
  },
}))
