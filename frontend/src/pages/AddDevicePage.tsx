import { FormEvent, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { api, getApiErrorMessage } from '../services/api'

const T = {
  uk: {
    step1Title: 'Підготовка пристрою',
    step1Desc: 'Перш ніж додати годівницю до застосунку, потрібно фізично налаштувати її по WiFi.',
    step1_1: 'Увімкніть годівницю',
    step1_2: 'Зачекайте ~10 секунд, поки пристрій запуститься',
    step1_3: 'Пристрій переходить у режим налаштування (AP-режим)',
    next: 'Далі →',
    back: '← Назад',
    cancel: 'Скасувати',
    step2Title: 'Підключення до годівниці',
    step2Desc: 'Пристрій зараз роздає власну WiFi-мережу. Підключіться до неї і налаштуйте домашній WiFi.',
    step2_1: 'Відкрийте WiFi-налаштування телефону або ноутбука',
    step2_2: 'Підключіться до мережі',
    step2_3: 'Пароль:',
    step2_4: 'Відкрийте браузер і перейдіть на',
    step2_5: 'Введіть назву та пароль домашнього WiFi і натисніть «Підключити»',
    step2_6: 'Пристрій підключиться до мережі й вийде з AP-режиму',
    connected: 'Пристрій підключено →',
    step3Title: 'Збережіть годівницю',
    step3Desc: 'Дайте годівниці назву, щоб відрізняти її від інших.',
    namePlaceholder: 'Вітальня',
    nameLabel: 'Назва годівниці',
    save: 'Зберегти годівницю',
    saving: 'Зберігаємо…',
    hint: 'Якщо залишити порожнім — буде «Годівниця 1», «Годівниця 2» тощо.',
    stepOf: (s: number) => `Крок ${s} з 3`,
  },
  en: {
    step1Title: 'Prepare the device',
    step1Desc: 'Before adding a feeder to the app, you need to set it up over WiFi.',
    step1_1: 'Turn on the feeder',
    step1_2: 'Wait ~10 seconds for the device to boot',
    step1_3: 'The device enters setup mode (AP mode)',
    next: 'Next →',
    back: '← Back',
    cancel: 'Cancel',
    step2Title: 'Connect to the feeder',
    step2Desc: 'The device is broadcasting its own WiFi network. Connect to it and configure your home WiFi.',
    step2_1: 'Open WiFi settings on your phone or laptop',
    step2_2: 'Connect to the network',
    step2_3: 'Password:',
    step2_4: 'Open a browser and go to',
    step2_5: 'Enter your home WiFi name and password, then tap "Connect"',
    step2_6: 'The device will connect to your network and exit AP mode',
    connected: 'Device connected →',
    step3Title: 'Save feeder',
    step3Desc: 'Give your feeder a name to tell it apart from others.',
    namePlaceholder: 'Living room',
    nameLabel: 'Feeder name',
    save: 'Save feeder',
    saving: 'Saving…',
    hint: 'Leave empty for "Feeder 1", "Feeder 2", etc.',
    stepOf: (s: number) => `Step ${s} of 3`,
  },
}

export function AddDevicePage() {
  const navigate = useNavigate()
  const lang = (localStorage.getItem('lang') ?? 'uk') as 'uk' | 'en'
  const t = T[lang] ?? T.uk

  const [step, setStep] = useState<1 | 2 | 3>(1)
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
      setError(getApiErrorMessage(err, 'Не вдалося додати годівницю'))
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <section className="devices-section">
      <div className="add-device-wizard">
        {/* Progress indicator */}
        <div className="wizard-step-indicator">
          {([1, 2, 3] as const).map((s) => (
            <span key={s} className={`wizard-dot${step >= s ? ' active' : ''}`} />
          ))}
          <span className="wizard-step-label">{t.stepOf(step)}</span>
        </div>

        {/* Step 1 */}
        {step === 1 && (
          <div className="wizard-card">
            <div className="wizard-header">
              <div className="wizard-icon">
                <svg width="40" height="40" viewBox="0 0 48 48" fill="none" xmlns="http://www.w3.org/2000/svg">
                  <circle cx="24" cy="24" r="23" stroke="#1976D2" strokeWidth="2" fill="#e8f1fb"/>
                  <path d="M24 13v10" stroke="#1976D2" strokeWidth="3" strokeLinecap="round"/>
                  <path d="M16.5 17.5A11 11 0 1 0 31.5 17.5" stroke="#1976D2" strokeWidth="3" strokeLinecap="round" fill="none"/>
                </svg>
              </div>
              <div className="wizard-header-text">
                <h2 className="wizard-title">{t.step1Title}</h2>
                <p className="wizard-desc">{t.step1Desc}</p>
              </div>
            </div>
            <ol className="wizard-steps-list">
              <li>{t.step1_1}</li>
              <li>{t.step1_2}</li>
              <li>{t.step1_3}</li>
            </ol>
            <div className="wizard-actions">
              <button className="wizard-btn-secondary" onClick={() => navigate('/devices')}>
                {t.cancel}
              </button>
              <button className="wizard-btn-primary" onClick={() => setStep(2)}>
                {t.next}
              </button>
            </div>
          </div>
        )}

        {/* Step 2 */}
        {step === 2 && (
          <div className="wizard-card">
            <div className="wizard-header">
              <div className="wizard-icon">
                <svg width="40" height="40" viewBox="0 0 48 48" fill="none" xmlns="http://www.w3.org/2000/svg">
                  <circle cx="24" cy="24" r="23" stroke="#1976D2" strokeWidth="2" fill="#e8f1fb"/>
                  <path d="M10 22c3.8-3.8 8.9-6 14-6s10.2 2.2 14 6" stroke="#1976D2" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
                  <path d="M14.5 26.5c2.5-2.5 5.9-4 9.5-4s7 1.5 9.5 4" stroke="#1976D2" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
                  <path d="M19 31c1.3-1.3 3-2 5-2s3.7.7 5 2" stroke="#1976D2" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
                  <circle cx="24" cy="36" r="2" fill="#1976D2"/>
                </svg>
              </div>
              <div className="wizard-header-text">
                <h2 className="wizard-title">{t.step2Title}</h2>
                <p className="wizard-desc">{t.step2Desc}</p>
              </div>
            </div>
            <ol className="wizard-steps-list">
              <li>{t.step2_1}</li>
              <li>
                {t.step2_2}: <code className="wizard-code">FishFeeder-XXXX</code>
              </li>
              <li>
                {t.step2_3} <code className="wizard-code">12345678</code>
              </li>
              <li>
                {t.step2_4}: <code className="wizard-code">192.168.4.1</code>
              </li>
              <li>{t.step2_5}</li>
              <li>{t.step2_6}</li>
            </ol>
            <div className="wizard-actions">
              <button className="wizard-btn-secondary" onClick={() => setStep(1)}>
                {t.back}
              </button>
              <button className="wizard-btn-primary" onClick={() => setStep(3)}>
                {t.connected}
              </button>
            </div>
          </div>
        )}

        {/* Step 3 */}
        {step === 3 && (
          <div className="wizard-card">
            <div className="wizard-header">
              <div className="wizard-icon">
                <svg width="40" height="40" viewBox="0 0 48 48" fill="none" xmlns="http://www.w3.org/2000/svg">
                  <circle cx="24" cy="24" r="23" stroke="#1976D2" strokeWidth="2" fill="#e8f1fb"/>
                  <path d="M14 25l7 7 13-14" stroke="#1976D2" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round"/>
                </svg>
              </div>
              <div className="wizard-header-text">
                <h2 className="wizard-title">{t.step3Title}</h2>
                <p className="wizard-desc">{t.step3Desc}</p>
              </div>
            </div>
            <form className="auth-form" onSubmit={(e) => void handleSubmit(e)}>
              <label>
                {t.nameLabel}
                <input
                  type="text"
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  placeholder={t.namePlaceholder}
                  maxLength={40}
                  autoFocus
                />
              </label>
              <p className="panel-hint" style={{ marginTop: 4 }}>{t.hint}</p>
              {error ? <div className="form-error">{error}</div> : null}
              <div className="wizard-actions">
                <button type="button" className="wizard-btn-secondary" onClick={() => setStep(2)}>
                  {t.back}
                </button>
                <button type="submit" className="wizard-btn-primary" disabled={submitting}>
                  {submitting ? t.saving : t.save}
                </button>
              </div>
            </form>
          </div>
        )}
      </div>
    </section>
  )
}
