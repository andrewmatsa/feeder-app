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
import { useSafeAreaInsets } from 'react-native-safe-area-context'
import { ChevronRightIcon, EditIcon, TrashIcon } from '../../src/components/ActionIcons'
import { AquariumScreen } from '../../src/components/AquariumScreen'
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
  const insets = useSafeAreaInsets()
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
      <AquariumScreen interactive>
        <View style={styles.center}>
          <ActivityIndicator size="large" color="#fff" />
        </View>
      </AquariumScreen>
    )
  }

  return (
    <AquariumScreen interactive>
      <View style={[styles.container, { paddingTop: insets.top + 20 }]}>
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
                style={({ pressed }) => [
                  styles.openButton,
                  openBlocked && styles.openButtonDisabled,
                  pressed && !openBlocked && styles.openButtonPressed,
                ]}
                onPress={() => router.push(`/devices/${device.id}`)}
                disabled={openBlocked}
              >
                <Text style={[styles.openButtonText, openBlocked && styles.openButtonTextDisabled]}>{T.open}</Text>
                <ChevronRightIcon color={openBlocked ? '#9ca3af' : '#fff'} size={15} />
              </Pressable>
              <View style={styles.cardActionsRight}>
                <Pressable
                  style={({ pressed }) => [styles.iconButton, pressed && styles.iconButtonPressed]}
                  onPress={() => startRename(device)}
                  hitSlop={6}
                >
                  <EditIcon color="#1f4a5c" />
                </Pressable>
                <Pressable
                  style={({ pressed }) => [styles.iconButton, styles.iconButtonDanger, pressed && styles.iconButtonDangerPressed]}
                  onPress={() => handleDelete(device)}
                  hitSlop={6}
                >
                  <TrashIcon color="#c1462b" />
                </Pressable>
              </View>
            </View>
          </View>
          )
        }}
      />

      <Pressable style={styles.logoutButton} onPress={() => void logout()}>
        <Text style={styles.logoutButtonText}>{T.logout}</Text>
      </Pressable>
      </View>
    </AquariumScreen>
  )
}

const styles = StyleSheet.create({
  container: { flex: 1, padding: 16 },
  center: { flex: 1, alignItems: 'center', justifyContent: 'center' },
  toolbar: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginBottom: 12 },
  title: { fontSize: 22, fontWeight: '700', color: '#fff6e4' },
  addButton: { backgroundColor: '#ec6a3b', borderRadius: 999, paddingHorizontal: 14, paddingVertical: 8 },
  addButtonText: { color: '#fff', fontWeight: '600', fontSize: 13 },
  error: { color: '#ffb199', marginBottom: 8, fontWeight: '600' },
  list: { gap: 12, paddingBottom: 24 },
  emptyContainer: { flexGrow: 1, justifyContent: 'center' },
  emptyState: { alignItems: 'center', gap: 16 },
  emptyText: { color: 'rgba(255,255,255,0.75)', fontSize: 15 },
  card: { borderWidth: 1, borderColor: '#eee', borderRadius: 12, padding: 16, marginBottom: 12, backgroundColor: '#fafafa' },
  badge: { fontSize: 12, marginBottom: 6, fontWeight: '600' },
  badgeOnline: { color: '#16a34a' },
  badgeOffline: { color: '#999' },
  cardTitle: { fontSize: 17, fontWeight: '600', marginBottom: 14 },
  renameRow: { flexDirection: 'row', alignItems: 'center', gap: 8, marginBottom: 14 },
  renameInput: { flex: 1, borderWidth: 1, borderColor: '#ddd', borderRadius: 6, paddingHorizontal: 8, paddingVertical: 6 },
  cardActions: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' },
  cardActionsRight: { flexDirection: 'row', alignItems: 'center', gap: 8 },
  openButton: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 4,
    backgroundColor: '#ec6a3b',
    borderRadius: 999,
    paddingHorizontal: 16,
    paddingVertical: 9,
    shadowColor: '#ec6a3b',
    shadowOpacity: 0.35,
    shadowRadius: 6,
    shadowOffset: { width: 0, height: 3 },
    elevation: 2,
  },
  openButtonPressed: { backgroundColor: '#d85c30' },
  openButtonText: { color: '#fff', fontWeight: '800', fontSize: 13 },
  openButtonDisabled: { backgroundColor: '#e5e7eb', shadowOpacity: 0 },
  openButtonTextDisabled: { color: '#9ca3af' },
  iconButton: {
    width: 34,
    height: 34,
    borderRadius: 17,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#eef2ff',
  },
  iconButtonPressed: { backgroundColor: '#dde5fb' },
  iconButtonDanger: { backgroundColor: '#fdeae4' },
  iconButtonDangerPressed: { backgroundColor: '#fad9cd' },
  smallButton: { backgroundColor: '#2563eb', borderRadius: 6, paddingHorizontal: 10, paddingVertical: 6 },
  smallButtonText: { color: '#fff', fontWeight: '600', fontSize: 12 },
  ghostButton: { backgroundColor: '#eee' },
  ghostButtonText: { color: '#333', fontWeight: '600', fontSize: 12 },
  logoutButton: {
    alignSelf: 'center',
    marginTop: 16,
    paddingVertical: 11,
    paddingHorizontal: 22,
    borderRadius: 999,
    backgroundColor: 'rgba(255,255,255,0.12)',
    borderWidth: 1.5,
    borderColor: 'rgba(255,177,153,0.55)',
  },
  logoutButtonText: { color: '#ffb199', fontWeight: '800', fontSize: 14 },
})
