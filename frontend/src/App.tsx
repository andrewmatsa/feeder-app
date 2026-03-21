import { useEffect, useState } from 'react'
import { api } from './services/api'
import { StatusResponse } from './types'
import './App.css'

function App() {
  const [status, setStatus] = useState<StatusResponse | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [feeding, setFeeding] = useState(false)

  const fetchStatus = async () => {
    try {
      setError(null)
      const data = await api.getStatus()
      setStatus(data)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load status')
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    fetchStatus()
    const interval = setInterval(fetchStatus, 5000) // Refresh every 5 seconds
    return () => clearInterval(interval)
  }, [])

  const handleFeed = async () => {
    try {
      setFeeding(true)
      await api.feedNow({ repeats: status?.feedRepeats || 1 })
      setTimeout(() => {
        fetchStatus()
        setFeeding(false)
      }, 1000)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Feeding error')
      setFeeding(false)
    }
  }

  const handleSetSpeed = async (speed: number) => {
    try {
      await api.setSpeed({ speed })
      fetchStatus()
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to set speed')
    }
  }

  const handleSetSchedule = async (times: string[]) => {
    try {
      await api.setSchedule({ times })
      fetchStatus()
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to set schedule')
    }
  }

  if (loading) {
    return (
      <div className="app">
        <div className="container">
          <div className="loading">Loading...</div>
        </div>
      </div>
    )
  }

  return (
    <div className="app">
      <div className="container">
        <header className="header">
          <h1>🐟 AquaFeed</h1>
          <p className="subtitle">Automatic fish feeder</p>
        </header>

        {error && (
          <div className="error-banner">
            ⚠️ {error}
          </div>
        )}

        {status && (
          <div className="status-grid">
            <div className="status-card">
              <h2>System status</h2>
              <div className="status-item">
                <span className="label">Battery:</span>
                <span className="value">
                  {status.batteryPercent}% ({status.batteryVoltage.toFixed(2)}V)
                </span>
              </div>
              <div className="status-item">
                <span className="label">Power saving mode:</span>
                <span className="value">
                  {status.powerSaveMode ? '✅ Enabled' : '❌ Disabled'}
                </span>
              </div>
              <div className="status-item">
                <span className="label">Servo angle:</span>
                <span className="value">{status.angle}°</span>
              </div>
              <div className="status-item">
                <span className="label">Speed:</span>
                <span className="value">{status.speed}</span>
              </div>
              <div className="status-item">
                <span className="label">Feed repeats:</span>
                <span className="value">{status.feedRepeats}</span>
              </div>
            </div>

            <div className="status-card">
              <h2>Feeding schedule</h2>
              <div className="schedule-list">
                {status.feedTimes.length > 0 ? (
                  status.feedTimes.map((time, index) => (
                    <div key={index} className="schedule-item">
                      🕐 {time}
                    </div>
                  ))
                ) : (
                  <div className="schedule-item">No schedule configured</div>
                )}
              </div>
            </div>

            <div className="status-card actions">
              <h2>Actions</h2>
              <button
                className="feed-button"
                onClick={handleFeed}
                disabled={feeding}
              >
                {feeding ? 'Feeding...' : '🍽️ Feed now'}
              </button>
            </div>
          </div>
        )}

        <footer className="footer">
          <p>AquaFeed v1.0.0</p>
        </footer>
      </div>
    </div>
  )
}

export default App

