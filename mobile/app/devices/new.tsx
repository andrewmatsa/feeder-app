import { Redirect, useRouter } from 'expo-router'
import { useEffect, useRef, useState } from 'react'
import { ActivityIndicator, Pressable, ScrollView, StyleSheet, Text, TextInput, View } from 'react-native'
import { useSafeAreaInsets } from 'react-native-safe-area-context'
import { AquariumScreen } from '../../src/components/AquariumScreen'
import { api, checkBackendReachable, getApiErrorMessage } from '../../src/services/api'
import {
  fetchProvisionStatus,
  probeDeviceReachable,
  submitWifiCredentials,
  type ProvisionStatus,
} from '../../src/services/deviceProvision'
import { useAuthStore } from '../../src/store/authStore'

const T = {
  choiceTitle: 'Яка це годівниця?',
  choiceDesc: 'Пристрій, який уже підключений до домашнього WiFi (наприклад, доданий раніше через веб-версію), не потребує повторного налаштування WiFi.',
  choiceNewTitle: 'Нова, ще не налаштована',
  choiceNewDesc: 'Щойно з коробки — потрібно підключити до домашнього WiFi',
  choiceExistingTitle: 'Уже підключена до WiFi',
  choiceExistingDesc: 'Наприклад, уже додана через веб-версію — просто зареєструвати в цьому акаунті',
  step1Title: 'Підготовка пристрою',
  step1Desc: 'Перш ніж додати годівницю до застосунку, потрібно фізично налаштувати її по WiFi.',
  step1_1: 'Увімкніть годівницю',
  step1_2: 'Зачекайте ~10 секунд, поки пристрій запуститься',
  step1_3: 'Пристрій переходить у режим налаштування (AP-режим)',
  next: 'Далі →',
  back: '← Назад',
  cancel: 'Скасувати',
  step2Title: 'Підключення до годівниці',
  step2Desc: 'Пристрій зараз роздає власну WiFi-мережу. Підключіться до неї, і форма нижче стане активною автоматично.',
  step2_1: 'Відкрийте WiFi-налаштування телефону',
  step2_2: 'Підключіться до мережі',
  step2_3: 'Пароль:',
  waitingForDevice: 'Очікуємо підключення до годівниці…',
  homeWifiSsid: 'Назва домашньої WiFi-мережі',
  homeWifiSsidPlaceholder: 'МійДомашнійWiFi',
  homeWifiPassword: 'Пароль домашньої WiFi',
  homeWifiPasswordPlaceholder: 'Введіть пароль',
  connectAction: 'Підключити',
  connecting: (ssid: string) => `Підключення до ${ssid}…`,
  connectFailed: 'Не вдалося підключитися. Перевірте пароль і спробуйте ще раз.',
  connectionUncertain: 'Не вдається перевірити стан підключення. Перегляньте екран годівниці — якщо там написано "Connected", натисніть "Пристрій підключено" нижче.',
  connectedToHome: (ssid: string) => `Підключено до ${ssid}!`,
  connected: 'Пристрій підключено →',
  step3Title: 'Збережіть годівницю',
  step3Desc: 'Дайте годівниці назву, щоб відрізняти її від інших.',
  namePlaceholder: 'Вітальня',
  nameLabel: 'Назва годівниці',
  save: 'Зберегти годівницю',
  saving: 'Зберігаємо…',
  hint: 'Якщо залишити порожнім — буде «Годівниця 1», «Годівниця 2» тощо.',
  awaitingNetwork: 'Очікуємо підключення телефону до домашнього WiFi…',
  stepOf: (s: number) => `Крок ${s} з 3`,
  addError: 'Не вдалося додати годівницю',
}

// Registering a device in the backend is just an account-scoped name record
// (see backend/device_service.py) — it has no dependency on the WiFi-pairing
// flow. So a device that's already on the home network shouldn't be routed
// through the AP-pairing steps at all; it only needs a name.
type Step = 'choice' | 1 | 2 | 3

export default function AddDeviceScreen() {
  const insets = useSafeAreaInsets()
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const router = useRouter()
  const [step, setStep] = useState<Step>('choice')
  const [name, setName] = useState('')
  const [error, setError] = useState<string | null>(null)
  const [submitting, setSubmitting] = useState(false)

  const [deviceReachable, setDeviceReachable] = useState(false)
  const [wifiSsid, setWifiSsid] = useState('')
  const [wifiPassword, setWifiPassword] = useState('')
  const [provisionStatus, setProvisionStatus] = useState<ProvisionStatus | null>(null)
  const [provisionError, setProvisionError] = useState<string | null>(null)
  const [provisionSubmitting, setProvisionSubmitting] = useState(false)
  const [awaitingNetworkHandoff, setAwaitingNetworkHandoff] = useState(false)
  const statusPollRef = useRef<ReturnType<typeof setInterval> | null>(null)
  const pollFailuresRef = useRef(0)

  const stopStatusPolling = () => {
    if (statusPollRef.current) {
      clearInterval(statusPollRef.current)
      statusPollRef.current = null
    }
  }

  // Step 2: while the phone hasn't joined the device's AP yet, poll for
  // reachability so the credentials form can appear automatically instead
  // of asking the user to guess when they're connected.
  useEffect(() => {
    if (step !== 2 || deviceReachable) return
    let cancelled = false
    const check = async () => {
      const reachable = await probeDeviceReachable()
      if (!cancelled && reachable) setDeviceReachable(true)
    }
    void check()
    const id = setInterval(check, 2000)
    return () => {
      cancelled = true
      clearInterval(id)
    }
  }, [step, deviceReachable])

  useEffect(() => stopStatusPolling, [])

  // After WiFi provisioning, the phone may still be mid-handoff between the
  // device's AP and the home network (OS reconnection isn't instant). Wait
  // for the backend to actually be reachable before letting the user hit
  // Save, so createDevice doesn't fire into a dead network and surface a
  // raw timeout.
  useEffect(() => {
    if (step !== 3 || !awaitingNetworkHandoff) return
    let cancelled = false
    const check = async () => {
      const reachable = await checkBackendReachable()
      if (!cancelled && reachable) setAwaitingNetworkHandoff(false)
    }
    void check()
    const id = setInterval(check, 1500)
    return () => {
      cancelled = true
      clearInterval(id)
    }
  }, [step, awaitingNetworkHandoff])

  const handleConnectWifi = async () => {
    setProvisionError(null)
    setProvisionSubmitting(true)
    try {
      await submitWifiCredentials(wifiSsid.trim(), wifiPassword)
      setProvisionStatus('pending')
      stopStatusPolling()
      pollFailuresRef.current = 0
      statusPollRef.current = setInterval(async () => {
        try {
          const data = await fetchProvisionStatus()
          pollFailuresRef.current = 0
          setProvisionError(null)
          setProvisionStatus(data.status)
          if (data.status === 'connected') {
            stopStatusPolling()
            setAwaitingNetworkHandoff(true)
            setTimeout(() => setStep(3), 1200)
          } else if (data.status === 'failed') {
            stopStatusPolling()
            setProvisionError(T.connectFailed)
          }
        } catch {
          // Can't tell from here whether the AP is gone because the device
          // succeeded (tears down its AP ~8s after connecting) or because
          // its station radio is scanning channels hunting for a bad SSID
          // (which can also make the AP flaky/unreachable for the phone for
          // a similar stretch) — both look identical from here: a dead poll.
          // Guessing "connected" from a timeout alone was wrong (a bad SSID
          // could sit unreachable for the same few seconds without ever
          // having succeeded). Surface a neutral hint instead of a false
          // positive; the OLED shows live status now, and the "already
          // connected" button below remains the honest way to skip ahead.
          pollFailuresRef.current += 1
          if (pollFailuresRef.current >= 5) {
            // Unlock the form again — otherwise the SSID/password fields and
            // the Connect button stay locked forever (both gate on
            // provisionStatus === 'pending'), leaving no way to retry.
            stopStatusPolling()
            setProvisionStatus(null)
            setProvisionError(T.connectionUncertain)
          }
        }
      }, 1000)
    } catch {
      setProvisionError(T.connectFailed)
    } finally {
      setProvisionSubmitting(false)
    }
  }

  const connectBlocked = !wifiSsid.trim() || !wifiPassword || provisionSubmitting || provisionStatus === 'pending'

  if (!isAuthenticated) {
    return <Redirect href="/login" />
  }

  const handleSubmit = async () => {
    setSubmitting(true)
    setError(null)
    try {
      const payload = name.trim() ? { name: name.trim() } : {}
      const device = await api.createDevice(payload)
      // Route straight into the dashboard rather than the list: opening it
      // fires the first /api/status fetch, which is what marks a freshly
      // added device online. The list's Open button stays blocked for
      // offline devices, so without this the device would never get its
      // first check.
      router.replace(`/devices/${device.id}`)
    } catch (err) {
      setError(getApiErrorMessage(err, T.addError))
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <AquariumScreen interactive>
      <ScrollView contentContainerStyle={[styles.scrollContent, { paddingTop: insets.top + 20 }]}>
      <View style={styles.container}>
      {step !== 'choice' && (
        <View style={styles.stepIndicator}>
          {([1, 2, 3] as const).map((s) => (
            <View key={s} style={[styles.dot, step >= s && styles.dotActive]} />
          ))}
          <Text style={styles.stepLabel}>{T.stepOf(step)}</Text>
        </View>
      )}

      {step === 'choice' && (
        <View style={styles.card}>
          <Text style={styles.cardTitle}>{T.choiceTitle}</Text>
          <Text style={styles.cardDesc}>{T.choiceDesc}</Text>

          <Pressable
            style={({ pressed }) => [styles.choiceCard, pressed && styles.choiceCardPressed]}
            onPress={() => setStep(1)}
          >
            <Text style={styles.choiceCardTitle}>{T.choiceNewTitle}</Text>
            <Text style={styles.choiceCardDesc}>{T.choiceNewDesc}</Text>
          </Pressable>

          <Pressable
            style={({ pressed }) => [styles.choiceCard, pressed && styles.choiceCardPressed]}
            onPress={() => setStep(3)}
          >
            <Text style={styles.choiceCardTitle}>{T.choiceExistingTitle}</Text>
            <Text style={styles.choiceCardDesc}>{T.choiceExistingDesc}</Text>
          </Pressable>

          <View style={styles.actions}>
            <Pressable style={styles.secondaryButton} onPress={() => router.replace('/devices')}>
              <Text style={styles.secondaryButtonText}>{T.cancel}</Text>
            </Pressable>
          </View>
        </View>
      )}

      {step === 1 && (
        <View style={styles.card}>
          <Text style={styles.cardTitle}>{T.step1Title}</Text>
          <Text style={styles.cardDesc}>{T.step1Desc}</Text>
          <View style={styles.list}>
            <Text style={styles.listItem}>1. {T.step1_1}</Text>
            <Text style={styles.listItem}>2. {T.step1_2}</Text>
            <Text style={styles.listItem}>3. {T.step1_3}</Text>
          </View>
          <View style={styles.actions}>
            <Pressable style={styles.secondaryButton} onPress={() => setStep('choice')}>
              <Text style={styles.secondaryButtonText}>{T.back}</Text>
            </Pressable>
            <Pressable style={styles.primaryButton} onPress={() => setStep(2)}>
              <Text style={styles.primaryButtonText}>{T.next}</Text>
            </Pressable>
          </View>
        </View>
      )}

      {step === 2 && (
        <View style={styles.card}>
          <Text style={styles.cardTitle}>{T.step2Title}</Text>
          <Text style={styles.cardDesc}>{T.step2Desc}</Text>
          <View style={styles.list}>
            <Text style={styles.listItem}>1. {T.step2_1}</Text>
            <Text style={styles.listItem}>
              2. {T.step2_2}: <Text style={styles.code}>FishFeeder-XXXX</Text>
            </Text>
            <Text style={styles.listItem}>
              3. {T.step2_3} <Text style={styles.code}>12345678</Text>
            </Text>
          </View>

          {!deviceReachable ? (
            <View style={styles.waitingRow}>
              <ActivityIndicator />
              <Text style={styles.waitingText}>{T.waitingForDevice}</Text>
            </View>
          ) : (
            <View style={styles.list}>
              <Text style={styles.label}>{T.homeWifiSsid}</Text>
              <TextInput
                style={styles.input}
                value={wifiSsid}
                onChangeText={setWifiSsid}
                placeholder={T.homeWifiSsidPlaceholder}
                autoCapitalize="none"
                editable={!provisionSubmitting && provisionStatus !== 'pending'}
              />
              <Text style={styles.label}>{T.homeWifiPassword}</Text>
              <TextInput
                style={styles.input}
                value={wifiPassword}
                onChangeText={setWifiPassword}
                placeholder={T.homeWifiPasswordPlaceholder}
                secureTextEntry
                editable={!provisionSubmitting && provisionStatus !== 'pending'}
              />

              {provisionStatus === 'pending' && (
                <View style={styles.waitingRow}>
                  <ActivityIndicator />
                  <Text style={styles.waitingText}>{T.connecting(wifiSsid)}</Text>
                </View>
              )}
              {provisionStatus === 'connected' && (
                <Text style={styles.success}>{T.connectedToHome(wifiSsid)}</Text>
              )}
              {provisionError && <Text style={styles.error}>{provisionError}</Text>}

              <Pressable
                style={[styles.primaryButton, styles.wifiConnectButton, connectBlocked && styles.wifiConnectButtonDisabled]}
                onPress={() => void handleConnectWifi()}
                disabled={connectBlocked}
              >
                {provisionSubmitting ? (
                  <ActivityIndicator color="#fff" />
                ) : (
                  <Text style={[styles.primaryButtonText, connectBlocked && styles.wifiConnectButtonTextDisabled]}>
                    {T.connectAction}
                  </Text>
                )}
              </Pressable>
            </View>
          )}

          <View style={styles.actions}>
            <Pressable style={styles.secondaryButton} onPress={() => setStep(1)}>
              <Text style={styles.secondaryButtonText}>{T.back}</Text>
            </Pressable>
            <Pressable style={styles.primaryButton} onPress={() => setStep(3)}>
              <Text style={styles.primaryButtonText}>{T.connected}</Text>
            </Pressable>
          </View>
        </View>
      )}

      {step === 3 && (
        <View style={styles.card}>
          <Text style={styles.cardTitle}>{T.step3Title}</Text>
          <Text style={styles.cardDesc}>{T.step3Desc}</Text>

          <Text style={styles.label}>{T.nameLabel}</Text>
          <TextInput
            style={styles.input}
            value={name}
            onChangeText={setName}
            placeholder={T.namePlaceholder}
            maxLength={40}
            autoFocus
          />
          <Text style={styles.hint}>{T.hint}</Text>

          {awaitingNetworkHandoff && (
            <View style={styles.waitingRow}>
              <ActivityIndicator />
              <Text style={styles.waitingText}>{T.awaitingNetwork}</Text>
            </View>
          )}

          {error && <Text style={styles.error}>{error}</Text>}

          <View style={styles.actions}>
            <Pressable style={styles.secondaryButton} onPress={() => setStep('choice')}>
              <Text style={styles.secondaryButtonText}>{T.back}</Text>
            </Pressable>
            <Pressable
              style={styles.primaryButton}
              onPress={() => void handleSubmit()}
              disabled={submitting || awaitingNetworkHandoff}
            >
              {submitting ? <ActivityIndicator color="#fff" /> : <Text style={styles.primaryButtonText}>{T.save}</Text>}
            </Pressable>
          </View>
        </View>
      )}
      </View>
      </ScrollView>
    </AquariumScreen>
  )
}

const styles = StyleSheet.create({
  scrollContent: { flexGrow: 1, padding: 16 },
  container: {
    backgroundColor: 'rgba(255,255,255,0.97)',
    borderRadius: 22,
    padding: 20,
    shadowColor: '#021420',
    shadowOpacity: 0.3,
    shadowRadius: 16,
    shadowOffset: { width: 0, height: 10 },
    elevation: 6,
  },
  stepIndicator: { flexDirection: 'row', alignItems: 'center', gap: 6, marginBottom: 20 },
  dot: { width: 8, height: 8, borderRadius: 4, backgroundColor: '#ddd' },
  dotActive: { backgroundColor: '#2563eb' },
  stepLabel: { marginLeft: 8, color: '#666', fontSize: 13 },
  card: { gap: 12 },
  cardTitle: { fontSize: 20, fontWeight: '700' },
  cardDesc: { color: '#555', fontSize: 14, lineHeight: 20 },
  list: { gap: 8, marginTop: 4 },
  listItem: { fontSize: 14, lineHeight: 20 },
  code: { fontFamily: 'monospace', backgroundColor: '#eef2ff', paddingHorizontal: 4, borderRadius: 4 },
  label: { fontSize: 14, color: '#444', marginTop: 8 },
  input: {
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 8,
    paddingHorizontal: 12,
    paddingVertical: 10,
    fontSize: 16,
  },
  hint: { color: '#888', fontSize: 12 },
  error: { color: '#c0392b' },
  success: { color: '#16a34a', fontWeight: '600' },
  waitingRow: { flexDirection: 'row', alignItems: 'center', gap: 10, marginTop: 4 },
  waitingText: { color: '#555', fontSize: 14 },
  wifiConnectButton: { alignSelf: 'flex-start', marginTop: 4 },
  wifiConnectButtonDisabled: { backgroundColor: '#e5e7eb' },
  wifiConnectButtonTextDisabled: { color: '#9ca3af' },
  actions: { flexDirection: 'row', justifyContent: 'flex-end', gap: 10, marginTop: 16 },
  secondaryButton: { paddingHorizontal: 16, paddingVertical: 12, borderRadius: 8, backgroundColor: '#f0f0f0' },
  secondaryButtonText: { fontWeight: '600', color: '#333' },
  primaryButton: { paddingHorizontal: 20, paddingVertical: 12, borderRadius: 8, backgroundColor: '#2563eb', minWidth: 100, alignItems: 'center' },
  primaryButtonText: { color: '#fff', fontWeight: '600' },
  choiceCard: {
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 10,
    padding: 14,
    gap: 4,
  },
  choiceCardPressed: { borderColor: '#2563eb', backgroundColor: '#eef2ff' },
  choiceCardTitle: { fontSize: 15, fontWeight: '700' },
  choiceCardDesc: { fontSize: 13, color: '#666' },
})
