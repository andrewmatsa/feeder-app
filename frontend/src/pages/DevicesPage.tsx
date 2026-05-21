import { useCallback, useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'
import { useUiStore } from '../store/uiStore'
import type { Device } from '../types'
import { isDeviceOnline } from '../utils/deviceStatus'

const T = {
  uk: {
    title: 'Мої годівниці',
    add: '+ Додати годівницю',
    addFirst: 'Додати першу годівницю',
    empty: 'Ще немає зареєстрованих годівниць.',
    open: 'Відкрити',
    rename: 'Перейменувати',
    delete: 'Видалити',
    save: 'Зберегти',
    cancel: 'Скасувати',
    online: '● Онлайн',
    offline: '○ Офлайн',
    loadError: 'Не вдалося завантажити годівниці',
    renameShort: 'Назва має містити щонайменше 2 символи',
    renameError: 'Не вдалося перейменувати',
    deleteError: 'Не вдалося видалити',
    deleteConfirm: (name: string) => `Видалити «${name}»?\n\nПристрій буде повністю видалено з акаунта. Усі налаштування годівниці скинуться. Цю дію неможливо скасувати.`,
  },
  en: {
    title: 'My feeders',
    add: '+ Add feeder',
    addFirst: 'Add first feeder',
    empty: 'No feeders registered yet.',
    open: 'Open',
    rename: 'Rename',
    delete: 'Delete',
    save: 'Save',
    cancel: 'Cancel',
    online: '● Online',
    offline: '○ Offline',
    loadError: 'Failed to load feeders',
    renameShort: 'Name must be at least 2 characters',
    renameError: 'Failed to rename',
    deleteError: 'Failed to delete',
    deleteConfirm: (name: string) => `Delete "${name}"?\n\nThe device will be permanently removed from your account. All feeder settings will be reset. This action cannot be undone.`,
  },
}

export function DevicesPage() {
  const lang = useUiStore(s => s.lang)
  const t = T[lang] ?? T.uk

  const [devices, setDevices] = useState<Device[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [renamingId, setRenamingId] = useState<string | null>(null)
  const [renameValue, setRenameValue] = useState('')

  const loadDevices = useCallback(async () => {
    try {
      setError(null)
      const list = await api.listDevices()
      setDevices(list)
    } catch (err) {
      setError(getApiErrorMessage(err, t.loadError))
    } finally {
      setLoading(false)
    }
  }, [t.loadError])

  useEffect(() => {
    void loadDevices()
  }, [loadDevices])

  const startRename = (device: Device) => {
    setRenamingId(device.id)
    setRenameValue(device.name)
  }

  const cancelRename = () => {
    setRenamingId(null)
    setRenameValue('')
  }

  const saveRename = async (deviceId: string) => {
    const trimmed = renameValue.trim()
    if (trimmed.length < 2) { setError(t.renameShort); return }
    try {
      setError(null)
      const updated = await api.updateDevice(deviceId, { name: trimmed })
      setDevices((prev) => prev.map((d) => (d.id === deviceId ? updated : d)))
      cancelRename()
    } catch (err) {
      setError(getApiErrorMessage(err, t.renameError))
    }
  }

  const handleDelete = async (device: Device) => {
    if (!window.confirm(t.deleteConfirm(device.name))) return
    try {
      setError(null)
      await api.deleteDevice(device.id)
      setDevices((prev) => prev.filter((d) => d.id !== device.id))
    } catch (err) {
      setError(getApiErrorMessage(err, t.deleteError))
    }
  }

  if (loading) {
    return (
      <section className="devices-section">
        <div className="devices-toolbar">
          <h2>{t.title}</h2>
          <span className="sk" style={{ width: 140, height: 36, borderRadius: 999 }} />
        </div>
        <div className="device-grid">
          {[1, 2].map((i) => (
            <div key={i} className="device-card">
              <span className="sk" style={{ width: '55%', height: 18, marginBottom: 10 }} />
              <span className="sk" style={{ width: '38%', height: 13, marginBottom: 20 }} />
              <div style={{ display: 'flex', gap: 8 }}>
                <span className="sk" style={{ width: 82, height: 36, borderRadius: 999 }} />
                <span className="sk" style={{ width: 116, height: 36, borderRadius: 999 }} />
                <span className="sk" style={{ width: 76, height: 36, borderRadius: 999 }} />
              </div>
            </div>
          ))}
        </div>
      </section>
    )
  }

  return (
    <section className="devices-section">
      <div className="devices-toolbar">
        <h2>{t.title}</h2>
        <Link to="/devices/new" className="primary-button link-button" style={{ padding: '6px 14px', fontSize: 12, boxShadow: 'none' }}>
          {t.add}
        </Link>
      </div>

      {error && <div className="error-banner">{error}</div>}

      {devices.length === 0 ? (
        <div className="empty-state">
          <p>{t.empty}</p>
          <Link to="/devices/new" className="primary-button link-button">
            {t.addFirst}
          </Link>
        </div>
      ) : (
        <div className="device-grid">
          {devices.map((device) => (
            <article key={device.id} className="device-card">
              {renamingId !== device.id && (
                <p className={`device-badge ${isDeviceOnline(device.lastSeen) ? 'online' : 'offline'}`}>
                  {isDeviceOnline(device.lastSeen) ? t.online : t.offline}
                </p>
              )}

              {renamingId === device.id ? (
                <div className="rename-row">
                  <input value={renameValue} onChange={(e) => setRenameValue(e.target.value)} maxLength={40} autoFocus />
                  <button type="button" className="small-button" onClick={() => void saveRename(device.id)}>{t.save}</button>
                  <button type="button" className="small-button ghost" onClick={cancelRename}>{t.cancel}</button>
                </div>
              ) : (
                <h3 className="device-card-title">{device.name}</h3>
              )}

              <div className="device-card-actions">
                <Link to={`/devices/${device.id}`} className="primary-button link-button" style={{ padding: '7px 20px', fontSize: 13 }}>
                  {t.open}
                </Link>
                <button type="button" className="icon-btn" title={t.rename} onClick={() => startRename(device)}>
                  <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/>
                    <path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/>
                  </svg>
                </button>
                <button type="button" className="icon-btn danger" title={t.delete} onClick={() => void handleDelete(device)}>
                  <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                    <polyline points="3 6 5 6 21 6"/>
                    <path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/>
                    <path d="M10 11v6M14 11v6"/>
                    <path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/>
                  </svg>
                </button>
              </div>
            </article>
          ))}
        </div>
      )}
    </section>
  )
}
