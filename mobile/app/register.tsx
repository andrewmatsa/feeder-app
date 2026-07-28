import { Link, Redirect, useRouter } from 'expo-router'
import { useState } from 'react'
import { ActivityIndicator, Pressable, ScrollView, StyleSheet, Text, TextInput, View } from 'react-native'
import { useSafeAreaInsets } from 'react-native-safe-area-context'
import { AquaMascot } from '../src/components/AquaMascot'
import { AquariumScreen } from '../src/components/AquariumScreen'
import { useAuthStore } from '../src/store/authStore'

const T = {
  title: 'Реєстрація',
  email: 'Email',
  password: 'Пароль',
  submit: 'Зареєструватися',
  submitting: 'Створюємо…',
  error: 'Помилка реєстрації',
  hasAccount: 'Вже є акаунт?',
  login: 'Увійти',
  tagline: 'Автоматична годівниця',
}

export default function RegisterScreen() {
  const insets = useSafeAreaInsets()
  const register = useAuthStore((s) => s.register)
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const router = useRouter()

  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [error, setError] = useState<string | null>(null)
  const [submitting, setSubmitting] = useState(false)

  if (isAuthenticated) {
    return <Redirect href="/devices" />
  }

  const handleSubmit = async () => {
    setSubmitting(true)
    setError(null)
    try {
      await register({ email, password })
      router.replace('/devices')
    } catch (err) {
      setError(err instanceof Error ? err.message : T.error)
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <AquariumScreen interactive>
      <ScrollView
        contentContainerStyle={[styles.scrollContent, { paddingTop: insets.top + 24 }]}
        keyboardShouldPersistTaps="handled"
      >
        <View style={styles.hero}>
          <AquaMascot size={168} />
          <View style={styles.wordmark}>
            <Text style={styles.wordmarkText}>
              <Text style={styles.wordAqua}>Aqua</Text>
              <Text style={styles.wordFeed}>Feed</Text>
            </Text>
            <Text style={styles.tagline}>{T.tagline}</Text>
          </View>
        </View>

        <View style={styles.container}>
          <Text style={styles.title}>{T.title}</Text>

          <Text style={styles.label}>{T.email}</Text>
          <TextInput
            style={styles.input}
            value={email}
            onChangeText={setEmail}
            autoCapitalize="none"
            keyboardType="email-address"
            autoComplete="email"
            placeholderTextColor="#94a8b0"
          />

          <Text style={styles.label}>{T.password}</Text>
          <TextInput
            style={styles.input}
            value={password}
            onChangeText={setPassword}
            secureTextEntry
            autoComplete="new-password"
            placeholderTextColor="#94a8b0"
          />

          {error && <Text style={styles.error}>{error}</Text>}

          <Pressable style={styles.button} onPress={() => void handleSubmit()} disabled={submitting}>
            {submitting ? <ActivityIndicator color="#fff" /> : <Text style={styles.buttonText}>{T.submit}</Text>}
          </Pressable>

          <View style={styles.switchRow}>
            <Text style={styles.switchText}>{T.hasAccount} </Text>
            <Link href="/login">
              <Text style={styles.link}>{T.login}</Text>
            </Link>
          </View>
        </View>
      </ScrollView>
    </AquariumScreen>
  )
}

const styles = StyleSheet.create({
  scrollContent: { flexGrow: 1, alignItems: 'center', paddingHorizontal: 20, paddingBottom: 40 },
  hero: { alignItems: 'center', marginBottom: 18 },
  wordmark: { marginTop: 8, alignItems: 'center' },
  wordmarkText: { fontSize: 34, fontWeight: '800', letterSpacing: 0.3 },
  wordAqua: { color: '#7fe3ff' },
  wordFeed: { color: '#fff6e4' },
  tagline: { marginTop: 2, fontSize: 13, fontWeight: '600', color: 'rgba(255,255,255,0.75)' },
  container: {
    width: '100%',
    maxWidth: 380,
    backgroundColor: 'rgba(255,255,255,0.97)',
    borderRadius: 22,
    padding: 24,
    shadowColor: '#021420',
    shadowOpacity: 0.3,
    shadowRadius: 16,
    shadowOffset: { width: 0, height: 10 },
    elevation: 6,
  },
  title: { fontSize: 24, fontWeight: '800', marginBottom: 20, color: '#05222f' },
  label: { fontSize: 13, fontWeight: '700', color: '#1f4a5c', marginBottom: 6, marginTop: 12 },
  input: {
    borderWidth: 1.5,
    borderColor: '#dcebf1',
    borderRadius: 12,
    paddingHorizontal: 14,
    paddingVertical: 11,
    fontSize: 15,
    fontWeight: '600',
    color: '#05222f',
    backgroundColor: '#f4fafc',
  },
  error: { color: '#c14a24', marginTop: 12, fontWeight: '700', fontSize: 13 },
  button: {
    backgroundColor: '#ec6a3b',
    borderRadius: 14,
    paddingVertical: 14,
    alignItems: 'center',
    marginTop: 22,
    shadowColor: '#ec6a3b',
    shadowOpacity: 0.4,
    shadowRadius: 10,
    shadowOffset: { width: 0, height: 6 },
    elevation: 4,
  },
  buttonText: { color: '#fff', fontSize: 16, fontWeight: '800' },
  switchRow: { flexDirection: 'row', justifyContent: 'center', marginTop: 18 },
  switchText: { color: '#6b8894', fontWeight: '600', fontSize: 13 },
  link: { color: '#1f8fb8', fontWeight: '800', fontSize: 13 },
})
