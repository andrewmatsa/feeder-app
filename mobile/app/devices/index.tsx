import { Link, Redirect, useRouter } from 'expo-router'
import { useCallback, useEffect, useState } from 'react'
import {
  ActivityIndicator,
  Alert,
  FlatList,
  Pressable,
  RefreshControl,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native'
import { AppHeader } from '../../src/components/AppHeader'
import { api, getApiErrorMessage } from '../../src/services/api'
import { registerForPushNotificationsAsync } from '../../src/services/pushNotifications'
import { useAuthStore } from '../../src/store/authStore'
import type { Device } from '../../src/types'
import { isDeviceOnline } from '../../src/utils/deviceStatus'

const T = {
  title: 'Мої годівниці',
  add: '+ Додати',
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
  retry: 'Спробувати ще раз',
  renameShort: 'Назва має містити щонайменше 2 символи',
  renameError: 'Не вдалося перейменувати',
  deleteError: 'Не вдалося видалити',
  deleteTitle: 'Видалити годівницю?',
  deleteConfirm: (name: string) =>
    `Пристрій «${name}» буде повністю видалено з акаунта. Усі налаштування годівниці скинуться. Цю дію неможливо скасувати.`,
  logout: 'Вийти',
}

export default function DevicesScreen() {
  const isAuthenticated = useAuthStore((s) => s.isAuthenticated)
  const logout = useAuthStore((s) => s.logout)
  const router = useRouter()

  const [devices, setDevices] = useState<Device[]>([])
  const [loading, setLoading] = useState(true)
  const [refreshing, setRefreshing] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const [renamingId, setRenamingId] = useState<string | null>(null)
  const [renameValue, setRenameValue] = useState('')

  const loadDevices = useCallback(async () => {
    try {
      setError(null)
      const list = await api.listDevices()
      setDevices(list)

      // last_seen only advances while a device's own dashboard is open and
      // actively polling — otherwise the online badge here would go stale
      // within a minute regardless of whether the feeder is actually up.
      // Ping each device once (best-effort) so the list reflects live state
      // without requiring the user to open every dashboard first. One-off
      // per list load/refresh, not a background poll, so it doesn't add
      // continuous firmware traffic.
      if (list.length > 0) {
        await Promise.allSettled(list.map((d) => api.getStatus(d.id)))
        const refreshed = await api.listDevices()
        setDevices(refreshed)
      }
    } catch (err) {
      setError(getApiErrorMessage(err, T.loadError))
    } finally {
      setLoading(false)
      setRefreshing(false)
    }
  }, [])

  useEffect(() => {
    void loadDevices()
  }, [loadDevices])

  // Best-effort: register this device for push alerts on every device the
  // user has. No-ops silently in Expo Go / without an EAS project — see
  // pushNotifications.ts. Runs once devices are known, not on every refresh.
  useEffect(() => {
    if (devices.length === 0) return
    void registerForPushNotificationsAsync().then((token) => {
      if (!token) return
      for (const device of devices) {
        void api.registerPushToken(device.id, token).catch(() => {})
      }
    })
  }, [devices])

  if (!isAuthenticated) {
    return <Redirect href="/login" />
  }

  const onRefresh = () => {
    setRefreshing(true)
    void loadDevices()
  }

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
      setError(T.renameShort)
      return
    }
    try {
      setError(null)
      const updated = await api.updateDevice(deviceId, { name: trimmed })
      setDevices((prev) => prev.map((d) => (d.id === deviceId ? updated : d)))
      cancelRename()
    } catch (err) {
      setError(getApiErrorMessage(err, T.renameError))
    }
  }

  const handleDelete = (device: Device) => {
    Alert.alert(T.deleteTitle, T.deleteConfirm(device.name), [
      { text: T.cancel, style: 'cancel' },
      {
        text: T.delete,
        style: 'destructive',
        onPress: async () => {
          try {
            setError(null)
            await api.deleteDevice(device.id)
            setDevices((prev) => prev.filter((d) => d.id !== device.id))
          } catch (err) {
            setError(getApiErrorMessage(err, T.deleteError))
          }
        },
      },
    ])
  }

  if (loading) {
    return (
      <View style={styles.screen}>
        <AppHeader subtitle={T.title} />
        <View style={styles.center}>
          <ActivityIndicator size="large" />
        </View>
      </View>
    )
  }

  return (
    <View style={styles.screen}>
      <AppHeader subtitle={T.title} />
      <View style={styles.container}>
      <View style={styles.toolbar}>
        <Text style={styles.title}>{T.title}</Text>
        <Link href="/devices/new" style={styles.addButton}>
          <Text style={styles.addButtonText}>{T.add}</Text>
        </Link>
      </View>

      {error && <Text style={styles.error}>{error}</Text>}

      <FlatList
        data={devices}
        keyExtractor={(d) => d.id}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
        contentContainerStyle={devices.length === 0 ? styles.emptyContainer : styles.list}
        ListEmptyComponent={
          error ? (
            <View style={styles.emptyState}>
              <Pressable style={styles.addButton} onPress={onRefresh}>
                <Text style={styles.addButtonText}>{T.retry}</Text>
              </Pressable>
            </View>
          ) : (
            <View style={styles.emptyState}>
              <Text style={styles.emptyText}>{T.empty}</Text>
              <Link href="/devices/new" style={styles.addButton}>
                <Text style={styles.addButtonText}>{T.addFirst}</Text>
              </Link>
            </View>
          )
        }
        renderItem={({ item: device }) => {
          const online = isDeviceOnline(device.lastSeen)
          const openBlocked = !online
          return (
          <View style={styles.card}>
            <Text style={[styles.badge, online ? styles.badgeOnline : styles.badgeOffline]}>
              {online ? T.online : T.offline}
            </Text>

            {renamingId === device.id ? (
              <View style={styles.renameRow}>
                <TextInput
                  style={styles.renameInput}
                  value={renameValue}
                  onChangeText={setRenameValue}
                  maxLength={40}
                  autoFocus
                />
                <Pressable style={styles.smallButton} onPress={() => void saveRename(device.id)}>
                  <Text style={styles.smallButtonText}>{T.save}</Text>
                </Pressable>
                <Pressable style={[styles.smallButton, styles.ghostButton]} onPress={cancelRename}>
                  <Text style={styles.ghostButtonText}>{T.cancel}</Text>
                </Pressable>
              </View>
            ) : (
              <Text style={styles.cardTitle}>{device.name}</Text>
            )}

            <View style={styles.cardActions}>
              <Pressable
                style={[styles.openButton, openBlocked && styles.openButtonDisabled]}
                onPress={() => router.push(`/devices/${device.id}`)}
                disabled={openBlocked}
              >
                <Text style={[styles.openButtonText, openBlocked && styles.openButtonTextDisabled]}>{T.open}</Text>
              </Pressable>
              <Pressable style={styles.iconButton} onPress={() => startRename(device)}>
                <Text>✏️</Text>
              </Pressable>
              <Pressable style={styles.iconButton} onPress={() => handleDelete(device)}>
                <Text>🗑️</Text>
              </Pressable>
            </View>
          </View>
          )
        }}
      />

      <Pressable style={styles.logoutButton} onPress={() => void logout()}>
        <Text style={styles.logoutButtonText}>{T.logout}</Text>
      </Pressable>
      </View>
    </View>
  )
}

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: '#fff' },
  container: { flex: 1, padding: 16 },
  center: { flex: 1, alignItems: 'center', justifyContent: 'center' },
  toolbar: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginBottom: 12 },
  title: { fontSize: 22, fontWeight: '700' },
  addButton: { backgroundColor: '#2563eb', borderRadius: 999, paddingHorizontal: 14, paddingVertical: 8 },
  addButtonText: { color: '#fff', fontWeight: '600', fontSize: 13 },
  error: { color: '#c0392b', marginBottom: 8 },
  list: { gap: 12, paddingBottom: 24 },
  emptyContainer: { flexGrow: 1, justifyContent: 'center' },
  emptyState: { alignItems: 'center', gap: 16 },
  emptyText: { color: '#666', fontSize: 15 },
  card: { borderWidth: 1, borderColor: '#eee', borderRadius: 12, padding: 16, marginBottom: 12, backgroundColor: '#fafafa' },
  badge: { fontSize: 12, marginBottom: 6, fontWeight: '600' },
  badgeOnline: { color: '#16a34a' },
  badgeOffline: { color: '#999' },
  cardTitle: { fontSize: 17, fontWeight: '600', marginBottom: 14 },
  renameRow: { flexDirection: 'row', alignItems: 'center', gap: 8, marginBottom: 14 },
  renameInput: { flex: 1, borderWidth: 1, borderColor: '#ddd', borderRadius: 6, paddingHorizontal: 8, paddingVertical: 6 },
  cardActions: { flexDirection: 'row', alignItems: 'center', gap: 8 },
  openButton: { backgroundColor: '#2563eb', borderRadius: 999, paddingHorizontal: 18, paddingVertical: 8 },
  openButtonText: { color: '#fff', fontWeight: '600', fontSize: 13 },
  openButtonDisabled: { backgroundColor: '#e5e7eb' },
  openButtonTextDisabled: { color: '#9ca3af' },
  iconButton: { padding: 8, borderRadius: 999, backgroundColor: '#f0f0f0' },
  smallButton: { backgroundColor: '#2563eb', borderRadius: 6, paddingHorizontal: 10, paddingVertical: 6 },
  smallButtonText: { color: '#fff', fontWeight: '600', fontSize: 12 },
  ghostButton: { backgroundColor: '#eee' },
  ghostButtonText: { color: '#333', fontWeight: '600', fontSize: 12 },
  logoutButton: { alignSelf: 'center', marginTop: 8, paddingVertical: 10 },
  logoutButtonText: { color: '#c0392b', fontWeight: '600' },
})
