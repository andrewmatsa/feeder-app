import { useEffect, useState } from 'react'
import { Link, useParams } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'
import type { Device, FeedTime, StatusResponse } from '../types'

function formatFeedTime(time: FeedTime): string {
  const formatted = `${String(time.hour).padStart(2, '0')}:${String(time.minute).padStart(2, '0')}`
  return time.repeats > 1 ? `${formatted} (${time.repeats}x)` : formatted
}

export function DeviceDashboardPage() {
  const { deviceId } = useParams<{ deviceId: string }>()
  const [device, setDevice] = useState<Device | null>(null)
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [feeding, setFeeding] = useState(false)

  useEffect(() => {
    if (!deviceId) return
    void api.getDevice(deviceId).then(setDevice).catch((err) => {
      setError(getApiErrorMessage(err, 'Акваріум не знайдено'))
    })
  }, [deviceId])

  const fetchStatus = async () => {
    try {
      setError(null)
      const data = await api.getStatus()
      setStatus(data)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося завантажити статус'))
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    void fetchStatus()
    const interval = setInterval(() => void fetchStatus(), 5000)
    return () => clearInterval(interval)
  }, [deviceId])

  const handleFeed = async () => {
    if (!device) return
    const confirmed = window.confirm(`Годувати: ${device.name}?`)
    if (!confirmed) return

    try {
      setFeeding(true)
      await api.feedNow({ repeats: status?.feedRepeats || 1 })
      setTimeout(() => {
        void fetchStatus()
        setFeeding(false)
      }, 1000)
    } catch (err) {
      setError(getApiErrorMessage(err, 'Помилка годування'))
      setFeeding(false)
    }
  }

  if (!deviceId) {
    return <div className="error-banner">Немає ID пристрою</div>
  }

  if (loading && !status) {
    return <div className="loading">Завантаження…</div>
  }

  return (
    <section className="devices-section">
      <Link to="/devices" className="back-link">
        ← Мої акваріуми
      </Link>

      <div className="device-dashboard-header">
        <h2>{device?.name ?? 'Акваріум'}</h2>
      </div>

      {error && <div className="error-banner">{error}</div>}

      {status && (
        <div className="status-grid">
          <div className="status-card">
            <h2>Статус системи</h2>
            <div className="status-item">
              <span className="label">Батарея:</span>
              <span className="value">
                {status.batteryPercent}% ({status.batteryVoltage.toFixed(2)}V)
              </span>
            </div>
            <div className="status-item">
              <span className="label">Режим економії:</span>
              <span className="value">{status.powerSaveMode ? 'Увімкнено' : 'Вимкнено'}</span>
            </div>
            <div className="status-item">
              <span className="label">Кут:</span>
              <span className="value">{status.angle}°</span>
            </div>
            <div className="status-item">
              <span className="label">Швидкість:</span>
              <span className="value">{status.speed}</span>
            </div>
          </div>

          <div className="status-card">
            <h2>Розклад годування</h2>
            <div className="schedule-list">
              {status.feedTimes.length > 0 ? (
                status.feedTimes.map((time, index) => (
                  <div key={index} className="schedule-item">
                    🕐 {formatFeedTime(time)}
                  </div>
                ))
              ) : (
                <div className="schedule-item">Розклад не налаштовано</div>
              )}
            </div>
          </div>

          <div className="status-card actions">
            <h2>Дії</h2>
            <button className="feed-button" onClick={() => void handleFeed()} disabled={feeding}>
              {feeding ? 'Годуємо…' : `🍽️ Годувати: ${device?.name ?? 'акваріум'}`}
            </button>
          </div>
        </div>
      )}
    </section>
  )
}
