import { FormEvent, useState } from 'react'
import { Link, Navigate, useNavigate } from 'react-router-dom'
import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'

const T = {
  uk: { title: 'Реєстрація', password: 'Пароль', submit: 'Зареєструватися', submitting: 'Створюємо…', error: 'Помилка реєстрації', hasAccount: 'Вже є акаунт?', login: 'Увійти' },
  en: { title: 'Register', password: 'Password', submit: 'Register', submitting: 'Creating…', error: 'Registration error', hasAccount: 'Already have an account?', login: 'Sign in' },
}

export function RegisterPage() {
  const register = useAuthStore((s) => s.register)
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const lang = useUiStore(s => s.lang)
  const t = T[lang] ?? T.uk
  const navigate = useNavigate()

  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [error, setError] = useState<string | null>(null)
  const [submitting, setSubmitting] = useState(false)

  if (isAuthenticated) {
    return <Navigate to="/devices" replace />
  }

  const handleSubmit = async (event: FormEvent) => {
    event.preventDefault()
    setSubmitting(true)
    setError(null)
    try {
      await register({ email, password })
      navigate('/devices', { replace: true })
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
          <input type="password" value={password} onChange={(e) => setPassword(e.target.value)} required minLength={6} autoComplete="new-password" />
        </label>
        {error && <div className="form-error">{error}</div>}
        <button type="submit" className="primary-button" disabled={submitting}>
          {submitting ? t.submitting : t.submit}
        </button>
      </form>
      <p className="auth-switch">
        {t.hasAccount} <Link to="/login">{t.login}</Link>
      </p>
    </div>
  )
}
