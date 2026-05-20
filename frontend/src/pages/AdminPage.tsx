import { Fragment, useCallback, useEffect, useMemo, useRef, useState } from 'react'
import { api, getApiErrorMessage } from '../services/api'
import type { AdminDevice, AdminFeedEvent, AdminStats, AdminUser, CreateUserRequest } from '../types'

function fmtDate(iso: string): string {
  return new Date(iso).toLocaleDateString('uk-UA', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
  })
}

function fmtActivity(iso: string | null): string {
  if (!iso) return '—'
  const d = new Date(iso)
  return d.toLocaleDateString('uk-UA', { day: '2-digit', month: '2-digit', year: 'numeric' })
    + ' ' + d.toLocaleTimeString('uk-UA', { hour: '2-digit', minute: '2-digit', second: '2-digit' })
}

function Sparkline({ data, id, color = '#667eea' }: { data: number[]; id: string; color?: string }) {
  if (data.length < 2) return null
  const W = 100
  const H = 36
  const max = Math.max(...data, 1)
  const pts: [number, number][] = data.map((v, i) => [
    (i / (data.length - 1)) * W,
    H - (v / max) * (H - 6) - 3,
  ])
  // Smooth cubic bezier path
  let linePath = `M ${pts[0][0]},${pts[0][1]}`
  for (let i = 1; i < pts.length; i++) {
    const dx = (pts[i][0] - pts[i - 1][0]) / 2.5
    linePath += ` C ${pts[i - 1][0] + dx},${pts[i - 1][1]} ${pts[i][0] - dx},${pts[i][1]} ${pts[i][0]},${pts[i][1]}`
  }
  const fillPath = `${linePath} L ${W},${H} L 0,${H} Z`
  const gradId = `sg-${id}`

  return (
    <svg
      viewBox={`0 0 ${W} ${H}`}
      preserveAspectRatio="none"
      style={{ width: '100%', height: 40, display: 'block', marginTop: 8 }}
      aria-hidden="true"
    >
      <defs>
        <linearGradient id={gradId} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stopColor={color} stopOpacity="0.35" />
          <stop offset="100%" stopColor={color} stopOpacity="0" />
        </linearGradient>
      </defs>
      <path d={fillPath} fill={`url(#${gradId})`} />
      <path d={linePath} fill="none" stroke={color} strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  )
}

function fmtSource(source: string): string {
  if (source === 'manual') return 'Вручну'
  if (source === 'scheduled') return 'Розклад'
  return source
}

interface DevicesRowProps {
  userId: string
  onDeviceDeleted: (userId: string) => void
}

function DevicesRow({ userId, onDeviceDeleted }: DevicesRowProps) {
  const [devices, setDevices] = useState<AdminDevice[] | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    api.listUserDevices(userId)
      .then(setDevices)
      .catch((err) => setError(getApiErrorMessage(err, 'Помилка завантаження')))
      .finally(() => setLoading(false))
  }, [userId])

  const handleDelete = async (deviceId: string, name: string) => {
    if (!window.confirm(`Видалити пристрій "${name}"?`)) return
    try {
      await api.deleteUserDevice(userId, deviceId)
      setDevices((prev) => prev?.filter((d) => d.id !== deviceId) ?? null)
      onDeviceDeleted(userId)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Помилка видалення пристрою'))
    }
  }

  if (loading) return <td colSpan={6} className="admin-devices-cell">Завантаження…</td>
  if (error) return <td colSpan={6} className="admin-devices-cell admin-error">{error}</td>
  if (!devices?.length) return <td colSpan={6} className="admin-devices-cell">Немає пристроїв</td>

  return (
    <td colSpan={6} className="admin-devices-cell">
      <table className="admin-devices-table">
        <thead>
          <tr>
            <th>Назва</th>
            <th>Додано</th>
            <th>Остання активність</th>
            <th></th>
          </tr>
        </thead>
        <tbody>
          {devices.map((d) => (
            <tr key={d.id}>
              <td>{d.name}</td>
              <td>{fmtDate(d.created_at)}</td>
              <td>{fmtActivity(d.last_seen)}</td>
              <td>
                <button
                  type="button"
                  className="admin-btn-danger-sm"
                  onClick={() => void handleDelete(d.id, d.name)}
                  title="Видалити пристрій"
                >
                  ×
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </td>
  )
}

interface AddUserFormProps {
  onCreated: (user: AdminUser) => void
  onCancel: () => void
}

function AddUserForm({ onCreated, onCancel }: AddUserFormProps) {
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [role, setRole] = useState<'admin' | 'user'>('user')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const emailRef = useRef<HTMLInputElement>(null)

  useEffect(() => {
    emailRef.current?.focus()
  }, [])

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)
    setError(null)
    try {
      const req: CreateUserRequest = { email, password, role }
      const user = await api.createAdminUser(req)
      onCreated(user)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Помилка створення'))
    } finally {
      setLoading(false)
    }
  }

  return (
    <form className="admin-add-form" onSubmit={(e) => void handleSubmit(e)}>
      <h3 className="admin-add-title">Новий користувач</h3>
      {error && <div className="admin-error">{error}</div>}
      <div className="admin-add-fields">
        <input
          ref={emailRef}
          type="email"
          className="admin-input"
          placeholder="Email"
          value={email}
          onChange={(e) => setEmail(e.target.value)}
          required
          disabled={loading}
        />
        <input
          type="password"
          className="admin-input"
          placeholder="Пароль"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          required
          minLength={6}
          disabled={loading}
        />
        <select
          className="admin-input"
          value={role}
          onChange={(e) => setRole(e.target.value as 'admin' | 'user')}
          disabled={loading}
        >
          <option value="user">Користувач</option>
          <option value="admin">Адмін</option>
        </select>
      </div>
      <div className="admin-add-actions">
        <button type="submit" className="admin-btn-primary" disabled={loading}>
          {loading ? 'Створення…' : 'Створити'}
        </button>
        <button type="button" className="admin-btn-secondary" onClick={onCancel} disabled={loading}>
          Скасувати
        </button>
      </div>
    </form>
  )
}

export function AdminPage() {
  const [users, setUsers] = useState<AdminUser[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [togglingId, setTogglingId] = useState<string | null>(null)
  const [deletingId, setDeletingId] = useState<string | null>(null)
  const [expandedId, setExpandedId] = useState<string | null>(null)
  const [showAddForm, setShowAddForm] = useState(false)

  const [stats, setStats] = useState<AdminStats | null>(null)
  const [statsLoading, setStatsLoading] = useState(true)
  const [statsError, setStatsError] = useState<string | null>(null)

  const [activeTab, setActiveTab] = useState<'users' | 'feedings'>('users')
  const [searchQuery, setSearchQuery] = useState('')

  const [feedEvents, setFeedEvents] = useState<AdminFeedEvent[]>([])
  const [feedEventsLoading, setFeedEventsLoading] = useState(false)
  const [feedEventsError, setFeedEventsError] = useState<string | null>(null)
  const [feedEventsOffset, setFeedEventsOffset] = useState(0)
  const [feedEventsHasMore, setFeedEventsHasMore] = useState(true)
  const FEED_EVENTS_LIMIT = 50

  const loadUsers = useCallback(async () => {
    try {
      setError(null)
      const data = await api.listAdminUsers()
      setUsers(data)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося завантажити користувачів'))
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    void loadUsers()
  }, [loadUsers])

  useEffect(() => {
    api.getAdminStats()
      .then(setStats)
      .catch((err) => setStatsError(getApiErrorMessage(err, 'Не вдалося завантажити статистику')))
      .finally(() => setStatsLoading(false))
  }, [])

  const loadFeedEvents = useCallback(async (offset: number, append: boolean) => {
    setFeedEventsLoading(true)
    setFeedEventsError(null)
    try {
      const data = await api.listAdminFeedEvents(FEED_EVENTS_LIMIT, offset)
      setFeedEvents((prev) => append ? [...prev, ...data] : data)
      setFeedEventsHasMore(data.length === FEED_EVENTS_LIMIT)
      setFeedEventsOffset(offset + data.length)
    } catch (err) {
      setFeedEventsError(getApiErrorMessage(err, 'Не вдалося завантажити журнал годувань'))
    } finally {
      setFeedEventsLoading(false)
    }
  }, []) // eslint-disable-line react-hooks/exhaustive-deps

  useEffect(() => {
    if (activeTab === 'feedings' && feedEvents.length === 0 && !feedEventsLoading) {
      void loadFeedEvents(0, false)
    }
  }, [activeTab]) // eslint-disable-line react-hooks/exhaustive-deps

  const handleRoleToggle = async (user: AdminUser) => {
    const newRole = user.role === 'admin' ? 'user' : 'admin'
    const label = newRole === 'admin' ? 'адміністратора' : 'користувача'
    if (!window.confirm(`Змінити роль ${user.email} на ${label}?`)) return
    setTogglingId(user.user_id)
    try {
      const updated = await api.updateUserRole(user.user_id, newRole)
      setUsers((prev) => prev.map((u) => (u.user_id === updated.user_id ? updated : u)))
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося змінити роль'))
    } finally {
      setTogglingId(null)
    }
  }

  const handleDeleteUser = async (user: AdminUser) => {
    if (!window.confirm(`Видалити користувача ${user.email}? Всі його пристрої також буде видалено.`)) return
    setDeletingId(user.user_id)
    try {
      await api.deleteAdminUser(user.user_id)
      setUsers((prev) => prev.filter((u) => u.user_id !== user.user_id))
      if (expandedId === user.user_id) setExpandedId(null)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося видалити користувача'))
    } finally {
      setDeletingId(null)
    }
  }

  const handleDeviceDeleted = (userId: string) => {
    setUsers((prev) =>
      prev.map((u) =>
        u.user_id === userId ? { ...u, device_count: Math.max(0, u.device_count - 1) } : u,
      ),
    )
  }

  const handleUserCreated = (user: AdminUser) => {
    setUsers((prev) => [user, ...prev])
    setShowAddForm(false)
  }

  const filteredUsers = searchQuery.trim()
    ? users.filter((u) => u.email.toLowerCase().includes(searchQuery.trim().toLowerCase()))
    : users

  const [feedSearch, setFeedSearch] = useState('')

  const filteredFeedEvents = useMemo(() => {
    const q = feedSearch.trim().toLowerCase()
    if (!q) return feedEvents
    return feedEvents.filter(
      (ev) => ev.user_email.toLowerCase().includes(q) || ev.device_name.toLowerCase().includes(q),
    )
  }, [feedEvents, feedSearch])

  if (loading) {
    return (
      <section className="admin-section">
        <div className="loading">Завантаження…</div>
      </section>
    )
  }

  return (
    <section className="admin-section">

      {/* Stats cards */}
      <div className="admin-stats-grid">
        {statsLoading ? (
          <div className="admin-stats-loading">Завантаження…</div>
        ) : statsError ? (
          <div className="admin-error-banner" style={{ gridColumn: '1 / -1' }}>{statsError}</div>
        ) : stats ? (
          <>
            <div className="admin-stat-card">
              <span className="admin-stat-value">{stats.total_users}</span>
              <span className="admin-stat-label">Користувачів</span>
              <Sparkline data={stats.users_sparkline} id="users" color="#667eea" />
            </div>
            <div className="admin-stat-card">
              <span className="admin-stat-value">{stats.total_devices}</span>
              <span className="admin-stat-label">Пристроїв</span>
              <Sparkline data={stats.devices_sparkline} id="devices" color="#06b6d4" />
            </div>
            <div className="admin-stat-card">
              <span className="admin-stat-value">{stats.feedings_today}</span>
              <span className="admin-stat-label">Годувань сьогодні</span>
              <Sparkline data={stats.feedings_sparkline} id="feedings" color="#10b981" />
            </div>
            <div className="admin-stat-card">
              <span className="admin-stat-value">{stats.active_users_week}</span>
              <span className="admin-stat-label">Активних за тиждень</span>
              <Sparkline data={stats.active_sparkline} id="active" color="#f59e0b" />
            </div>
          </>
        ) : null}
      </div>

      {/* Header */}
      <div className="admin-header">
        <div>
          <h2 className="admin-title">Адміністрування</h2>
          <p className="admin-subtitle">Керування акаунтами та пристроями</p>
        </div>
        {activeTab === 'users' && (
          <button
            type="button"
            className="admin-btn-primary"
            onClick={() => setShowAddForm((v) => !v)}
          >
            {showAddForm ? 'Скасувати' : '+ Додати користувача'}
          </button>
        )}
      </div>

      {/* Tab bar */}
      <div className="admin-tab-bar">
        <button
          type="button"
          className={`admin-tab-btn${activeTab === 'users' ? ' active' : ''}`}
          onClick={() => setActiveTab('users')}
        >
          Користувачі
        </button>
        <button
          type="button"
          className={`admin-tab-btn${activeTab === 'feedings' ? ' active' : ''}`}
          onClick={() => setActiveTab('feedings')}
        >
          Годування
        </button>
      </div>

      {/* Users tab */}
      {activeTab === 'users' && (
        <>
          {showAddForm && (
            <AddUserForm onCreated={handleUserCreated} onCancel={() => setShowAddForm(false)} />
          )}

          {error && <div className="admin-error-banner">{error}</div>}

          <div className="admin-search-row">
            <input
              type="search"
              className="admin-input admin-search-input"
              placeholder="Пошук за email…"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>

          <div className="admin-table-wrapper">
            <table className="admin-table">
              <thead>
                <tr>
                  <th>Email</th>
                  <th>Реєстрація</th>
                  <th>Пристроїв</th>
                  <th>Остання активність</th>
                  <th>Роль</th>
                  <th>Статус</th>
                  <th></th>
                </tr>
              </thead>
              <tbody>
                {filteredUsers.map((user) => (
                  <Fragment key={user.user_id}>
                    <tr className={expandedId === user.user_id ? 'admin-row-expanded' : ''}>
                      <td className="admin-cell-email" data-label="Email">{user.email}</td>
                      <td data-label="Реєстрація">{fmtDate(user.created_at)}</td>
                      <td className="admin-cell-center" data-label="Пристроїв">{user.device_count}</td>
                      <td data-label="Остання активність">{fmtActivity(user.last_activity)}</td>
                      <td data-label="Роль">
                        <button
                          type="button"
                          className={`role-badge ${user.role === 'admin' ? 'role-admin' : 'role-user'}`}
                          onClick={() => void handleRoleToggle(user)}
                          disabled={togglingId === user.user_id}
                          title="Натисніть, щоб змінити роль"
                        >
                          {user.role === 'admin' ? 'Адмін' : 'Користувач'}
                        </button>
                      </td>
                      <td data-label="Статус">
                        <span className={`status-badge ${user.is_active ? 'status-active' : 'status-inactive'}`}>
                          {user.is_active ? 'Активний' : 'Неактивний'}
                        </span>
                      </td>
                      <td className="admin-cell-actions">
                        {user.device_count > 0 && (
                          <button
                            type="button"
                            className="admin-btn-icon"
                            onClick={() => setExpandedId((v) => (v === user.user_id ? null : user.user_id))}
                            title="Переглянути пристрої"
                          >
                            {expandedId === user.user_id ? '▲' : '▼'}
                          </button>
                        )}
                        <button
                          type="button"
                          className="admin-btn-danger"
                          onClick={() => void handleDeleteUser(user)}
                          disabled={deletingId === user.user_id}
                          title="Видалити користувача"
                        >
                          ×
                        </button>
                      </td>
                    </tr>
                    {expandedId === user.user_id && (
                      <tr className="admin-devices-row">
                        <DevicesRow userId={user.user_id} onDeviceDeleted={handleDeviceDeleted} />
                      </tr>
                    )}
                  </Fragment>
                ))}
                {filteredUsers.length === 0 && (
                  <tr>
                    <td colSpan={7} className="admin-cell-empty">
                      {searchQuery ? 'Нічого не знайдено' : 'Немає користувачів'}
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        </>
      )}

      {/* Feedings tab */}
      {activeTab === 'feedings' && (
        <>
          {feedEventsError && <div className="admin-error-banner">{feedEventsError}</div>}
          <div className="admin-search-row">
            <input
              type="search"
              className="admin-input admin-search-input"
              placeholder="Фільтр за email або пристроєм…"
              value={feedSearch}
              onChange={(e) => setFeedSearch(e.target.value)}
            />
          </div>

          <div className="admin-table-wrapper">
            <table className="admin-table">
              <thead>
                <tr>
                  <th>Час</th>
                  <th>Користувач</th>
                  <th>Пристрій</th>
                  <th>Кількість</th>
                  <th>Джерело</th>
                </tr>
              </thead>
              <tbody>
                {filteredFeedEvents.map((ev) => (
                  <tr key={ev.id}>
                    <td data-label="Час">{fmtActivity(ev.created_at)}</td>
                    <td data-label="Користувач" className="admin-cell-email">{ev.user_email}</td>
                    <td data-label="Пристрій">{ev.device_name}</td>
                    <td data-label="Кількість" className="admin-cell-center">{ev.repeats}</td>
                    <td data-label="Джерело">{fmtSource(ev.source)}</td>
                  </tr>
                ))}
                {filteredFeedEvents.length === 0 && !feedEventsLoading && (
                  <tr>
                    <td colSpan={5} className="admin-cell-empty">
                      {feedSearch ? 'Нічого не знайдено' : 'Немає записів'}
                    </td>
                  </tr>
                )}
                {feedEventsLoading && (
                  <tr>
                    <td colSpan={5} className="admin-cell-empty">Завантаження…</td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
          {feedEventsHasMore && !feedEventsLoading && (
            <div className="admin-load-more-row">
              <button
                type="button"
                className="admin-btn-secondary"
                onClick={() => void loadFeedEvents(feedEventsOffset, true)}
              >
                Завантажити ще
              </button>
            </div>
          )}
        </>
      )}

    </section>
  )
}
