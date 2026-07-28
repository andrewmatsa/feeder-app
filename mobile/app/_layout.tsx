import { Stack } from 'expo-router'
import { useEffect, useState } from 'react'
import { ActivityIndicator, View } from 'react-native'
import { SafeAreaProvider } from 'react-native-safe-area-context'
import { useAuthStore } from '../src/store/authStore'

export default function RootLayout() {
  const hydrate = useAuthStore((s) => s.hydrate)
  const hydrated = useAuthStore((s) => s.hydrated)
  const [hydrating, setHydrating] = useState(true)

  useEffect(() => {
    hydrate().finally(() => setHydrating(false))
  }, [hydrate])

  if (hydrating || !hydrated) {
    return (
      <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
        <ActivityIndicator size="large" />
      </View>
    )
  }

  return (
    <SafeAreaProvider>
      <Stack screenOptions={{ headerShown: false }}>
        <Stack.Screen name="index" />
        <Stack.Screen name="login" />
        <Stack.Screen name="register" />
        <Stack.Screen name="devices/index" />
        <Stack.Screen name="devices/new" />
        <Stack.Screen name="devices/[deviceId]" />
      </Stack>
    </SafeAreaProvider>
  )
}
