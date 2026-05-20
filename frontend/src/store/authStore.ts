import { create } from 'zustand'
import { api, authStorage, getApiErrorMessage, setAccessToken } from '../services/api'
import type { LoginRequest, RegisterRequest } from '../types'

const ROLE_KEY = 'aquafeed_role'

const roleStorage = {
  get: () => localStorage.getItem(ROLE_KEY) as 'admin' | 'user' | null,
  save: (role: string) => localStorage.setItem(ROLE_KEY, role),
  clear: () => localStorage.removeItem(ROLE_KEY),
}

interface AuthState {
  email: string | null
  role: 'admin' | 'user' | null
  isAuthenticated: boolean
  isAdmin: boolean
  login: (request: LoginRequest) => Promise<void>
  register: (request: RegisterRequest) => Promise<void>
  logout: () => Promise<void>
  hydrate: () => void
}

export const useAuthStore = create<AuthState>((set) => ({
  email: null,
  role: roleStorage.get(),
  isAuthenticated: !!authStorage.getToken(),
  isAdmin: roleStorage.get() === 'admin',

  hydrate: () => {
    setAccessToken(authStorage.getToken())
    const role = roleStorage.get()
    set({
      isAuthenticated: !!authStorage.getToken(),
      role,
      isAdmin: role === 'admin',
    })
  },

  login: async (request) => {
    try {
      const data = await api.login(request)
      roleStorage.save(data.role)
      set({
        email: data.email,
        role: data.role as 'admin' | 'user',
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
      set({
        email: data.email,
        role: data.role as 'admin' | 'user',
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
    set({ email: null, role: null, isAuthenticated: false, isAdmin: false })
  },
}))
