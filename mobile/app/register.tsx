import { Link, Redirect, useRouter } from 'expo-router'
import { useState } from 'react'
import { ActivityIndicator, Pressable, StyleSheet, Text, TextInput, View } from 'react-native'
import { AppHeader } from '../src/components/AppHeader'
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
}

export default function RegisterScreen() {
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
    <View style={styles.screen}>
      <AppHeader subtitle={T.title} />
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
        />

        <Text style={styles.label}>{T.password}</Text>
        <TextInput
          style={styles.input}
          value={password}
          onChangeText={setPassword}
          secureTextEntry
          autoComplete="new-password"
        />

        {error && <Text style={styles.error}>{error}</Text>}

        <Pressable style={styles.button} onPress={() => void handleSubmit()} disabled={submitting}>
          {submitting ? <ActivityIndicator color="#fff" /> : <Text style={styles.buttonText}>{T.submit}</Text>}
        </Pressable>

        <View style={styles.switchRow}>
          <Text>{T.hasAccount} </Text>
          <Link href="/login">
            <Text style={styles.link}>{T.login}</Text>
          </Link>
        </View>
      </View>
    </View>
  )
}

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: '#fff' },
  container: { flex: 1, justifyContent: 'center', padding: 24 },
  title: { fontSize: 28, fontWeight: '700', marginBottom: 24 },
  label: { fontSize: 14, color: '#444', marginBottom: 4, marginTop: 12 },
  input: {
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 8,
    paddingHorizontal: 12,
    paddingVertical: 10,
    fontSize: 16,
  },
  error: { color: '#c0392b', marginTop: 12 },
  button: {
    backgroundColor: '#2563eb',
    borderRadius: 8,
    paddingVertical: 14,
    alignItems: 'center',
    marginTop: 24,
  },
  buttonText: { color: '#fff', fontSize: 16, fontWeight: '600' },
  switchRow: { flexDirection: 'row', justifyContent: 'center', marginTop: 20 },
  link: { color: '#2563eb', fontWeight: '600' },
})
