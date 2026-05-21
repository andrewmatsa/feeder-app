import { FormEvent, useState } from 'react'
import { Link, Navigate, useLocation, useNavigate } from 'react-router-dom'
import { api } from '../services/api'
import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'

const T = {
  uk: { title: 'Вхід', password: 'Пароль', submit: 'Увійти', submitting: 'Входимо…', error: 'Помилка входу', noAccount: 'Немає акаунта?', register: 'Зареєструватися' },
  en: { title: 'Sign in', password: 'Password', submit: 'Sign in', submitting: 'Signing in…', error: 'Login error', noAccount: 'No account?', register: 'Register' },
}

export function LoginPage() {
  const login = useAuthStore((s) => s.login)
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const lang = useUiStore(s => s.lang)
  const t = T[lang] ?? T.uk
  const navigate = useNavigate()
  const location = useLocation()
  const from = (location.state as { from?: string } | null)?.from ?? '/devices'

  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [error, setError] = useState<string | null>(null)
  const [submitting, setSubmitting] = useState(false)

  if (isAuthenticated) {
    return <Navigate to={from} replace />
  }

  const handleSubmit = async (event: FormEvent) => {
    event.preventDefault()
    setSubmitting(true)
    setError(null)
    try {
      await login({ email, password })
      const devices = await api.listDevices().catch(() => [])
      if (devices.length > 0) {
        navigate(`/devices/${devices[0].id}`, { replace: true })
      } else {
        navigate(from, { replace: true })
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : t.error)
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <div className="auth-panel">
      <h2>{t.title}</h2>
      <form className="auth-form" onSubmit={(e) => void handleSubmit(e)}>
        <label>
          Email
          <input type="email" value={email} onChange={(e) => setEmail(e.target.value)} required autoComplete="email" />
        </label>
        <label>
          {t.password}
          <input type="password" value={password} onChange={(e) => setPassword(e.target.value)} required autoComplete="current-password" />
        </label>
        {error && <div className="form-error">{error}</div>}
        <button type="submit" className="primary-button" disabled={submitting}>
          {submitting ? t.submitting : t.submit}
        </button>
      </form>
      <p className="auth-switch">
        {t.noAccount} <Link to="/register">{t.register}</Link>
      </p>
    </div>
  )
}
