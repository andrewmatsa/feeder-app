import { FormEvent, useMemo, useRef, useState } from 'react'
import { Link, Navigate, useLocation, useNavigate } from 'react-router-dom'
import { api } from '../services/api'
import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'
import './LoginPage.css'

const T = {
  uk: {
    title: 'Вхід',
    password: 'Пароль',
    submit: 'Увійти',
    submitting: 'Входимо…',
    error: 'Помилка входу',
    noAccount: 'Немає акаунта?',
    register: 'Зареєструватися',
    tagline: 'Автоматична годівниця',
    fishAlt: 'Стилізована рибка AquaFeed',
  },
  en: {
    title: 'Sign in',
    password: 'Password',
    submit: 'Sign in',
    submitting: 'Signing in…',
    error: 'Login error',
    noAccount: 'No account?',
    register: 'Register',
    tagline: 'Automatic fish feeder',
    fishAlt: 'Stylized AquaFeed fish',
  },
}

// Fixed-but-varied ambient bubbles rising behind the card — generated once
// per mount so they don't all move in lockstep.
function useBubbleField(count: number) {
  return useMemo(
    () =>
      Array.from({ length: count }, (_, i) => ({
        left: `${(i * 137.5) % 100}%`,
        size: 6 + ((i * 53) % 22),
        duration: 9 + ((i * 7) % 10),
        delay: -((i * 3.3) % 12),
        drift: `${((i % 5) - 2) * 14}px`,
      })),
    [count],
  )
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
  const [darting, setDarting] = useState(false)
  const dartTimer = useRef<ReturnType<typeof setTimeout> | null>(null)

  const bubbles = useBubbleField(14)

  if (isAuthenticated) {
    return <Navigate to={from} replace />
  }

  const handleFishClick = () => {
    setDarting(false)
    // restart the animation even if clicked mid-dart
    requestAnimationFrame(() => setDarting(true))
    if (dartTimer.current) clearTimeout(dartTimer.current)
    dartTimer.current = setTimeout(() => setDarting(false), 900)
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
    <div className="lp2-page">
      <div className="lp2-ocean-bg" aria-hidden="true">
        <div className="lp2-ray lp2-ray-1" />
        <div className="lp2-ray lp2-ray-2" />
        {bubbles.map((b, i) => (
          <span
            key={i}
            className="lp2-bubble"
            style={{
              left: b.left,
              width: b.size,
              height: b.size,
              animationDuration: `${b.duration}s`,
              animationDelay: `${b.delay}s`,
              // @ts-expect-error custom property read by the keyframes
              '--drift': b.drift,
            }}
          />
        ))}

        <svg className="lp2-seaweed-field" viewBox="0 0 400 150" preserveAspectRatio="none" xmlns="http://www.w3.org/2000/svg">
          <path className="lp2-weed lp2-weed-a" d="M28,150 C18,120 40,108 26,84 C14,64 34,48 24,26" fill="none" stroke="#0c7a52" strokeWidth="9" strokeLinecap="round" />
          <path className="lp2-weed lp2-weed-b" d="M60,150 C72,116 48,100 62,72 C74,48 54,34 66,10" fill="none" stroke="#0f9463" strokeWidth="7" strokeLinecap="round" />
          <path className="lp2-weed lp2-weed-c" d="M336,150 C324,114 348,96 332,68 C318,44 340,30 328,8" fill="none" stroke="#0c7a52" strokeWidth="8" strokeLinecap="round" />
          <path className="lp2-weed lp2-weed-d" d="M370,150 C382,122 360,104 374,78 C386,56 366,40 378,18" fill="none" stroke="#0f9463" strokeWidth="6" strokeLinecap="round" />
          <path className="lp2-weed lp2-weed-e" d="M200,150 C190,124 210,110 198,88 C188,68 206,54 196,36" fill="none" stroke="#0a6845" strokeWidth="6" strokeLinecap="round" opacity="0.8" />
        </svg>
      </div>

      <div className="lp2-hero">
        <div
          className={`lp2-fish-wrap${darting ? ' lp2-dart' : ''}`}
          onClick={handleFishClick}
          role="button"
          tabIndex={0}
          aria-label={t.fishAlt}
          onKeyDown={(e) => { if (e.key === 'Enter' || e.key === ' ') handleFishClick() }}
        >
          <svg viewBox="0 0 200 140" xmlns="http://www.w3.org/2000/svg">
            <defs>
              <linearGradient id="lp2BodyGrad" x1="0" y1="0" x2="0" y2="1">
                <stop offset="0%" stopColor="#6fd8f5" />
                <stop offset="55%" stopColor="#2ea3cf" />
                <stop offset="100%" stopColor="#166f96" />
              </linearGradient>
              <radialGradient id="lp2BellyGrad" cx="40%" cy="30%" r="75%">
                <stop offset="0%" stopColor="#fffaf0" />
                <stop offset="100%" stopColor="#ffe9c2" />
              </radialGradient>
            </defs>

            {/* tail — two lobes fanning off the body's right edge */}
            <path
              className="lp2-tail"
              d="M156,72
                 C168,50 186,34 199,32
                 C204,32 203,40 198,46
                 C190,54 186,64 186,72
                 C186,80 190,90 198,98
                 C203,104 204,112 199,112
                 C186,110 168,94 156,72 Z"
              fill="#0e5876"
            />

            {/* pectoral fin — behind the body so only the trailing edge peeks out */}
            <path
              className="lp2-fin"
              d="M100,98 C90,102 82,116 90,126 C102,124 108,108 106,98 Z"
              fill="#0f5872"
            />

            {/* body — one clean rounded silhouette, no freehand kinks */}
            <ellipse cx="106" cy="72" rx="58" ry="50" fill="url(#lp2BodyGrad)" />

            {/* belly patch */}
            <ellipse cx="110" cy="94" rx="40" ry="26" fill="url(#lp2BellyGrad)" />

            {/* dorsal fin — three even scallops along the crown */}
            <path
              d="M82,26 C86,8 94,7 98,24 C102,6 111,6 115,24 C119,7 127,8 131,26 Z"
              fill="#0f5872"
            />

            {/* gill mark */}
            <path d="M86,52 C90,60 90,70 85,78" fill="none" stroke="#0d4a63" strokeWidth="2.5" strokeLinecap="round" opacity="0.5" />

            {/* scale hints, back/upper-right of the body */}
            <g stroke="#8fe3ff" strokeWidth="1.6" fill="none" opacity="0.35" strokeLinecap="round">
              <path d="M124,42 q6,6 0,12" />
              <path d="M138,50 q6,6 0,12" />
              <path d="M130,60 q6,6 0,12" />
              <path d="M144,66 q6,6 0,12" />
            </g>

            {/* mouth */}
            <path d="M42,70 C46,75 53,75 57,71" fill="none" stroke="#0d2b3d" strokeWidth="3.4" strokeLinecap="round" />

            {/* mouth bubbles */}
            <g fill="#eafcff">
              <circle className="lp2-mouth-bubble" cx="33" cy="58" r="3" />
              <circle className="lp2-mouth-bubble" cx="28" cy="48" r="2.2" />
              <circle className="lp2-mouth-bubble" cx="24" cy="38" r="4" />
            </g>

            {/* eye */}
            <circle cx="68" cy="52" r="16" fill="#fffdf6" />
            <circle cx="72" cy="52" r="8.5" fill="#132a35" />
            <circle cx="75.5" cy="47.5" r="3" fill="#fff" />

            {/* eyelid (blinks) */}
            <circle className="lp2-eyelid" cx="68" cy="52" r="16.5" fill="#2ea3cf" />
          </svg>
        </div>

        <div className="lp2-wordmark">
          <span className="lp2-word lp2-word-aqua">Aqua</span><span className="lp2-word lp2-word-feed">Feed</span>
          <span className="lp2-tagline">{t.tagline}</span>
        </div>
      </div>

      <div className="lp2-card">
        <h2>{t.title}</h2>
        <form className="lp2-form" onSubmit={(e) => void handleSubmit(e)}>
          <label>
            Email
            <input type="email" value={email} onChange={(e) => setEmail(e.target.value)} required autoComplete="email" />
          </label>
          <label>
            {t.password}
            <input type="password" value={password} onChange={(e) => setPassword(e.target.value)} required autoComplete="current-password" />
          </label>
          {error && <div className="lp2-error">{error}</div>}
          <button type="submit" className="lp2-submit" disabled={submitting}>
            {submitting ? t.submitting : t.submit}
          </button>
        </form>
        <p className="lp2-switch">
          {t.noAccount} <Link to="/register">{t.register}</Link>
        </p>
      </div>
    </div>
  )
}
