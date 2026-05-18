import { FormEvent, useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'

export function AddDevicePage() {
  const navigate = useNavigate()
  const [name, setName] = useState('')
  const [error, setError] = useState<string | null>(null)
  const [submitting, setSubmitting] = useState(false)

  const handleSubmit = async (event: FormEvent) => {
    event.preventDefault()
    setSubmitting(true)
    setError(null)
    try {
      const payload = name.trim() ? { name: name.trim() } : {}
      const device = await api.createDevice(payload)
      navigate(`/devices/${device.id}`, { replace: true })
    } catch (err) {
      setError(getApiErrorMessage(err, 'Не вдалося додати акваріум'))
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <section className="devices-section">
      <Link to="/devices" className="back-link">
        ← До списку
      </Link>
      <div className="auth-panel">
        <h2>Новий акваріум</h2>
        <p className="panel-hint">
          Вкажіть назву, щоб відрізнити годівницю, наприклад «Вітальня» або «Спальня». Якщо
          залишити порожнім — буде «Акваріум 1», «Акваріум 2» тощо.
        </p>
        <form className="auth-form" onSubmit={(e) => void handleSubmit(e)}>
          <label>
            Назва акваріума
            <input
              type="text"
              value={name}
              onChange={(e) => setName(e.target.value)}
              placeholder="Вітальня"
              maxLength={40}
            />
          </label>
          {error ? <div className="form-error">{error}</div> : null}
          <button type="submit" className="primary-button" disabled={submitting}>
            {submitting ? 'Додаємо…' : 'Додати'}
          </button>
        </form>
      </div>
    </section>
  )
}
