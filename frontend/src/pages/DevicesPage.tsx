import { useCallback, useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'
import type { Device } from '../types'
import { formatLastSeen, isDeviceOnline } from '../utils/deviceStatus'

export function DevicesPage() {
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
      setError(getApiErrorMessage(err, 'Не вдалося завантажити акваріуми'))
    } finally {
      setLoading(false)
    }
  }, [])

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
    if (trimmed.length < 2) {
      setError('Назва має містити щонайменше 2 символи')
      return
    }
    try {
      setError(null)
      const updated = await api.updateDevice(deviceId, { name: trimmed })
      setDevices((prev) => prev.map((d) => (d.id === deviceId ? updated : d)))
      cancelRename()
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося перейменувати'))
    }
  }

  const handleDelete = async (device: Device) => {
    const confirmed = window.confirm(`Видалити «${device.name}» з акаунта?`)
    if (!confirmed) return
    try {
      setError(null)
      await api.deleteDevice(device.id)
      setDevices((prev) => prev.filter((d) => d.id !== device.id))
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося видалити'))
    }
  }

  if (loading) {
    return <div className="loading">Завантаження…</div>
  }

  return (
    <section className="devices-section">
      <div className="devices-toolbar">
        <h2>Мої акваріуми</h2>
        <Link to="/devices/new" className="primary-button link-button">
          + Додати акваріум
        </Link>
      </div>

      {error && <div className="error-banner">{error}</div>}

      {devices.length === 0 ? (
        <div className="empty-state">
          <p>Ще немає зареєстрованих годівниць.</p>
          <Link to="/devices/new" className="primary-button link-button">
            Додати перший акваріум
          </Link>
        </div>
      ) : (
        <div className="device-grid">
          {devices.map((device) => (
            <article key={device.id} className="device-card">
              {renamingId === device.id ? (
                <div className="rename-row">
                  <input
                    value={renameValue}
                    onChange={(e) => setRenameValue(e.target.value)}
                    maxLength={40}
                    autoFocus
                  />
                  <button type="button" className="small-button" onClick={() => void saveRename(device.id)}>
                    Зберегти
                  </button>
                  <button type="button" className="small-button ghost" onClick={cancelRename}>
                    Скасувати
                  </button>
                </div>
              ) : (
                <h3 className="device-card-title">{device.name}</h3>
              )}

              <p className={`device-badge ${isDeviceOnline(device.lastSeen) ? 'online' : 'offline'}`}>
                {isDeviceOnline(device.lastSeen) ? '● Онлайн' : '○ Офлайн'}
                <span className="device-last-seen">{formatLastSeen(device.lastSeen)}</span>
              </p>

              <div className="device-card-actions">
                <Link to={`/devices/${device.id}`} className="primary-button link-button">
                  Відкрити
                </Link>
                <button type="button" className="small-button ghost" onClick={() => startRename(device)}>
                  Перейменувати
                </button>
                <button type="button" className="small-button danger" onClick={() => void handleDelete(device)}>
                  Видалити
                </button>
              </div>
            </article>
          ))}
        </div>
      )}
    </section>
  )
}
