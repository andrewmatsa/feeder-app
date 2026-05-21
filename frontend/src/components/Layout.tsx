import { useRef, useState, useEffect } from 'react'
import { Link, Outlet, useLocation } from 'react-router-dom'
import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'
import { authStorage } from '../services/api'
import { APP_VERSION } from '../version'
import '../App.css'

const NAV_T = {
  uk: { devices: 'Мої годівниці', admin: 'Адмін', logout: 'Вийти' },
  en: { devices: 'My feeders', admin: 'Admin', logout: 'Sign out' },
}

export function Layout() {
  const { isAdmin, logout } = useAuthStore()
  const pageSubtitle = useUiStore(s => s.pageSubtitle)
  const location = useLocation()
  const isAuthenticated = !!authStorage.getToken()
  const onDeviceList = location.pathname === '/devices' || location.pathname === '/devices/new'
  const [menuOpen, setMenuOpen] = useState(false)
  const lang = useUiStore(s => s.lang)
  const T = NAV_T[lang] ?? NAV_T.uk

  const illustrationRef = useRef<HTMLDivElement>(null)
  const tailBurstTimer = useRef<ReturnType<typeof setTimeout> | null>(null)
  const escapeTimer = useRef<ReturnType<typeof setTimeout> | null>(null)

  useEffect(() => { setMenuOpen(false) }, [location.pathname])

  const handleFishClick = (e: React.MouseEvent) => {
    e.stopPropagation()
    const illustration = illustrationRef.current
    if (!illustration || illustration.classList.contains('is-escaping')) return

    const tail = illustration.querySelector('.hero-fish-tail') as SVGElement | null
    illustration.classList.add('is-escaping')

    if (tail) {
      tail.classList.remove('tail-burst')
      void (tail as unknown as { offsetWidth: number }).offsetWidth
      tail.classList.add('tail-burst')
      if (tailBurstTimer.current) clearTimeout(tailBurstTimer.current)
      tailBurstTimer.current = setTimeout(() => {
        tail.classList.remove('tail-burst')
        tailBurstTimer.current = null
      }, 950)
    }

    if (escapeTimer.current) clearTimeout(escapeTimer.current)
    escapeTimer.current = setTimeout(() => {
      illustration.classList.remove('is-escaping')
      escapeTimer.current = null
    }, 2150)
  }

  return (
    <div className="app">
      <header className="header">
        <div className="header-inner">
          <div className="header-brand">
            <div className="app-illustration" ref={illustrationRef} onClick={handleFishClick}>
              <svg className="hero-svg-fish" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg">
                <path className="hero-fish-body" d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4" fill="#728389"/>
                <g fill="#8d9ba3">
                  <path className="hero-fish-tail" d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"/>
                  <ellipse cx="33.4" cy="35.3" rx="2.2" ry="3.2"/>
                  <ellipse cx="37.6" cy="39.2" rx="1.2" ry="2.5"/>
                  <ellipse cx="39.9" cy="36" rx=".6" ry="1.7"/>
                </g>
                <g fill="#75d6ff">
                  <ellipse className="hero-bubble hero-bubble-1" cx="5.3" cy="44" rx="1.7" ry="1.8"/>
                  <ellipse className="hero-bubble hero-bubble-2" cx="6.3" cy="23.4" rx="4.3" ry="4.5"/>
                  <ellipse className="hero-bubble hero-bubble-3" cx="12.8" cy="10.3" rx="8" ry="8.3"/>
                </g>
                <ellipse cx="18.7" cy="38.5" rx="7.1" ry="7.4" fill="#fcfcfa"/>
                <ellipse className="hero-fish-eye" cx="18.7" cy="38.5" rx="4.9" ry="5.1" fill="#29251c"/>
              </svg>
            </div>
            <div className="header-brand-text">
              <span className="header-title">AquaFeed</span>
              <span className="header-sub">{pageSubtitle}</span>
            </div>
          </div>

          {isAuthenticated && (
            <>
              <nav className="top-nav">
                <Link to="/devices" className={`nav-link${onDeviceList ? ' active' : ''}`}>
                  {T.devices}
                </Link>
                {isAdmin && (
                  <Link
                    to="/admin"
                    className={`nav-link${location.pathname.startsWith('/admin') ? ' active' : ''}`}
                  >
                    {T.admin}
                  </Link>
                )}
                <button type="button" className="nav-button" onClick={() => void logout()}>
                  {T.logout}
                </button>
              </nav>

              <button
                type="button"
                className={`burger-btn${menuOpen ? ' open' : ''}`}
                onClick={() => setMenuOpen(v => !v)}
                aria-label="Меню"
              >
                {menuOpen
                  ? <svg width="20" height="20" viewBox="0 0 20 20" fill="none"><line x1="3" y1="3" x2="17" y2="17" stroke="#374151" strokeWidth="2" strokeLinecap="round"/><line x1="17" y1="3" x2="3" y2="17" stroke="#374151" strokeWidth="2" strokeLinecap="round"/></svg>
                  : <svg width="20" height="20" viewBox="0 0 20 20" fill="none"><line x1="3" y1="5" x2="17" y2="5" stroke="#374151" strokeWidth="2" strokeLinecap="round"/><line x1="3" y1="10" x2="17" y2="10" stroke="#374151" strokeWidth="2" strokeLinecap="round"/><line x1="3" y1="15" x2="17" y2="15" stroke="#374151" strokeWidth="2" strokeLinecap="round"/></svg>
                }
              </button>

              {menuOpen && (
                <div className="burger-menu" onClick={() => setMenuOpen(false)}>
                  <Link to="/devices" className={`burger-item${onDeviceList ? ' active' : ''}`}>
                    {T.devices}
                  </Link>
                  {isAdmin && (
                    <Link
                      to="/admin"
                      className={`burger-item${location.pathname.startsWith('/admin') ? ' active' : ''}`}
                    >
                      {T.admin}
                    </Link>
                  )}
                  <button type="button" className="burger-item burger-logout" onClick={() => void logout()}>
                    {T.logout}
                  </button>
                </div>
              )}
            </>
          )}
        </div>
      </header>

      <div className="container">
        <Outlet />
      </div>

      <footer className="footer">
        <span>AquaFeed v{APP_VERSION}</span>
      </footer>
    </div>
  )
}
