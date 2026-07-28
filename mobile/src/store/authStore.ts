import * as SecureStore from 'expo-secure-store'
import { create } from 'zustand'
import { api, authStorage, getApiErrorMessage, setAccessToken, setSessionExpiredHandler } from '../services/api'
import type { LoginRequest, RegisterRequest } from '../types'

const ROLE_KEY = 'aquafeed_role'
const EMAIL_KEY = 'aquafeed_email'
const CREATED_AT_KEY = 'aquafeed_created_at'

const roleStorage = {
  get: () => SecureStore.getItemAsync(ROLE_KEY) as Promise<'admin' | 'user' | null>,
  save: (role: string) => SecureStore.setItemAsync(ROLE_KEY, role),
  clear: () => SecureStore.deleteItemAsync(ROLE_KEY),
}

interface AuthState {
  hydrated: boolean
  email: string | null
  role: 'admin' | 'user' | null
  createdAt: string | null
  isAuthenticated: boolean
  isAdmin: boolean
  login: (request: LoginRequest) => Promise<void>
  register: (request: RegisterRequest) => Promise<void>
  logout: () => Promise<void>
  hydrate: () => Promise<void>
  /** Local-only reset for a session the server already considers dead
   * (refresh token expired/invalid) — unlike logout(), never calls the API. */
  forceLogout: () => void
}

export const useAuthStore = create<AuthState>((set) => {
  setSessionExpiredHandler(() => {
    set({ email: null, role: null, createdAt: null, isAuthenticated: false, isAdmin: false })
  })

  return {
  hydrated: false,
  email: null,
  role: null,
  createdAt: null,
  isAuthenticated: false,
  isAdmin: false,

  hydrate: async () => {
    const [token, role, email, createdAt] = await Promise.all([
      authStorage.getToken(),
      roleStorage.get(),
      SecureStore.getItemAsync(EMAIL_KEY),
      SecureStore.getItemAsync(CREATED_AT_KEY),
    ])
    setAccessToken(token)
    set({
      hydrated: true,
      isAuthenticated: !!token,
      role,
      isAdmin: role === 'admin',
      email,
      createdAt,
    })
  },

  login: async (request) => {
    try {
      const data = await api.login(request)
      await roleStorage.save(data.role)
      if (data.email) await SecureStore.setItemAsync(EMAIL_KEY, data.email)
      if (data.created_at) await SecureStore.setItemAsync(CREATED_AT_KEY, data.created_at)
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
      await roleStorage.save(data.role)
      if (data.email) await SecureStore.setItemAsync(EMAIL_KEY, data.email)
      if (data.created_at) await SecureStore.setItemAsync(CREATED_AT_KEY, data.created_at)
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
    await roleStorage.clear()
    await SecureStore.deleteItemAsync(EMAIL_KEY)
    await SecureStore.deleteItemAsync(CREATED_AT_KEY)
    set({ email: null, role: null, createdAt: null, isAuthenticated: false, isAdmin: false })
  },

  forceLogout: () => {
    void roleStorage.clear()
    void SecureStore.deleteItemAsync(EMAIL_KEY)
    void SecureStore.deleteItemAsync(CREATED_AT_KEY)
    set({ email: null, role: null, createdAt: null, isAuthenticated: false, isAdmin: false })
  },
  }
})
