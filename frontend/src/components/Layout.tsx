import { Link, Outlet, useLocation } from 'react-router-dom'
import { useAuthStore } from '../store/authStore'
import { authStorage } from '../services/api'
import { APP_VERSION } from '../version'
import '../App.css'

export function Layout() {
  const { logout } = useAuthStore()
  const location = useLocation()
  const isAuthenticated = !!authStorage.getToken()
  const onDeviceList = location.pathname === '/devices' || location.pathname === '/devices/new'

  return (
    <div className="app">
      <div className="container">
        <header className="header">
          <h1>🐟 AquaFeed</h1>
          <p className="subtitle">Керування годівницями</p>
          {isAuthenticated && (
            <nav className="top-nav">
              <Link
                to="/devices"
                className={onDeviceList ? 'nav-link active' : 'nav-link'}
              >
                Мої акваріуми
              </Link>
              <button type="button" className="nav-button" onClick={() => void logout()}>
                Вийти
              </button>
            </nav>
          )}
        </header>

        <Outlet />

        <footer className="footer">
          <p>AquaFeed v{APP_VERSION}</p>
        </footer>
      </div>
    </div>
  )
}
