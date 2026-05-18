import { useRef, useState } from 'react'
import { TRANSLATIONS, type Lang } from '../translations'
import './LoginPage.css'

interface Props {
  onSuccess: (token: string, refreshToken: string) => void
}

export function LoginPage({ onSuccess }: Props) {
  const [lang, setLang] = useState<Lang>(() => (localStorage.getItem('aq_lang') as Lang) || 'uk')
  const [mode, setMode] = useState<'login' | 'register'>('login')
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const T = TRANSLATIONS[lang]

  const illustrationRef = useRef<HTMLDivElement>(null)
  const tailRef = useRef<SVGPathElement>(null)
  const escapingRef = useRef(false)

  const switchLang = (l: Lang) => {
    setLang(l)
    localStorage.setItem('aq_lang', l)
  }

  const handleFishClick = () => {
    const el = illustrationRef.current
    const tail = tailRef.current
    if (!el || !tail || escapingRef.current) return

    escapingRef.current = true
    el.classList.add('is-escaping')
    tail.classList.remove('tail-burst')
    void tail.offsetWidth
    tail.classList.add('tail-burst')

    setTimeout(() => tail.classList.remove('tail-burst'), 950)
    setTimeout(() => {
      el.classList.remove('is-escaping')
      escapingRef.current = false
    }, 2150)
  }

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)
    setError(null)
    try {
      const res = await fetch(`/auth/${mode}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password }),
      })
      const data = await res.json()
      if (!res.ok) {
        setError(data.detail || 'Something went wrong')
        return
      }
      onSuccess(data.access_token, data.refresh_token)
    } catch {
      setError(T.loginNetworkError)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="lp-shell-outer">
      <div className="lp-lang-row">
        <button
          type="button"
          className={`lp-lang-btn ${lang === 'uk' ? 'active' : ''}`}
          onClick={() => switchLang('uk')}
        >UK</button>
        <button
          type="button"
          className={`lp-lang-btn ${lang === 'en' ? 'active' : ''}`}
          onClick={() => switchLang('en')}
        >EN</button>
      </div>

      <div className="lp-shell">
        <div className="lp-hero-header">
          <div className="app-illustration" ref={illustrationRef}>
            <svg
              className="hero-svg-fish"
              viewBox="0 0 64 64"
              xmlns="http://www.w3.org/2000/svg"
              role="img"
              aria-label="Стилізована рибка"
              onClick={handleFishClick}
            >
              <path
                className="hero-fish-body"
                d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4"
                fill="#728389"
              />
              <g fill="#8d9ba3">
                <path
                  className="hero-fish-tail"
                  ref={tailRef}
                  d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"
                />
                <ellipse cx="33.4" cy="35.3" rx="2.2" ry="3.2" />
                <ellipse cx="37.6" cy="39.2" rx="1.2" ry="2.5" />
                <ellipse cx="39.9" cy="36" rx=".6" ry="1.7" />
              </g>
              <g fill="#75d6ff">
                <ellipse className="hero-bubble hero-bubble-1" cx="5.3" cy="44" rx="1.7" ry="1.8" />
                <ellipse className="hero-bubble hero-bubble-2" cx="6.3" cy="23.4" rx="4.3" ry="4.5" />
                <ellipse className="hero-bubble hero-bubble-3" cx="12.8" cy="10.3" rx="8" ry="8.3" />
              </g>
              <ellipse cx="18.7" cy="38.5" rx="7.1" ry="7.4" fill="#fcfcfa" />
              <ellipse className="hero-fish-eye" cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c" />
            </svg>
          </div>
          <div className="lp-app-heading">
            <div className="lp-app-title">AquaFeed</div>
            <div className="lp-app-subtitle">{T.loginSubtitle}</div>
          </div>
        </div>

        <div className="lp-card">
          <div className="lp-tabs">
            <button
              type="button"
              className={`lp-tab-btn ${mode === 'login' ? 'active' : ''}`}
              onClick={() => { setMode('login'); setError(null) }}
            >
              {T.loginSignIn}
            </button>
            <button
              type="button"
              className={`lp-tab-btn ${mode === 'register' ? 'active' : ''}`}
              onClick={() => { setMode('register'); setError(null) }}
            >
              {T.loginRegister}
            </button>
          </div>

          <form onSubmit={handleSubmit}>
            <label className="lp-label" htmlFor="lp-email">{T.loginEmail}</label>
            <input
              id="lp-email"
              className="lp-input"
              type="email"
              value={email}
              onChange={e => setEmail(e.target.value)}
              placeholder="you@example.com"
              required
              autoComplete="email"
            />

            <label className="lp-label" htmlFor="lp-password">{T.loginPassword}</label>
            <input
              id="lp-password"
              className="lp-input"
              type="password"
              value={password}
              onChange={e => setPassword(e.target.value)}
              placeholder="••••••••"
              required
              autoComplete={mode === 'login' ? 'current-password' : 'new-password'}
            />

            <button className="lp-submit" type="submit" disabled={loading}>
              {loading ? T.loginLoading : mode === 'login' ? T.loginSignIn : T.loginRegister}
            </button>
          </form>

          {error && <div className="lp-error">{error}</div>}
        </div>
      </div>
    </div>
  )
}
