import { useAuthStore } from '../store/authStore'
import { useUiStore } from '../store/uiStore'

const T = {
  uk: { logout: 'Вийти з акаунту' },
  en: { logout: 'Sign out' },
}

export function SettingsPage() {
  const { email, logout } = useAuthStore()
  const lang = useUiStore(s => s.lang)
  const t = T[lang] ?? T.uk

  return (
    <section className="devices-section">
      <div className="settings-card">
        <button className="settings-logout-btn" onClick={() => void logout()}>
          {t.logout}
        </button>
        {email && <p className="settings-account-hint">{email}</p>}
      </div>
    </section>
  )
}
