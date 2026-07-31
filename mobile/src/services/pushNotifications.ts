import Constants from 'expo-constants'
import * as Notifications from 'expo-notifications'
import { Platform } from 'react-native'

Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowBanner: true,
    shouldShowList: true,
    shouldPlaySound: true,
    shouldSetBadge: false,
  }),
})

// Returns null (never throws) when push isn't available — e.g. Expo Go on SDK
// 53+ (remote push notifications require a development/standalone build), no
// EAS projectId configured yet, permission denied, or running on a simulator.
export async function registerForPushNotificationsAsync(): Promise<string | null> {
  try {
    if (Platform.OS === 'android') {
      await Notifications.setNotificationChannelAsync('default', {
        name: 'default',
        importance: Notifications.AndroidImportance.DEFAULT,
      })
    }

    const existing = await Notifications.getPermissionsAsync()
    let status = existing.status
    if (status !== 'granted') {
      const requested = await Notifications.requestPermissionsAsync()
      status = requested.status
    }
    if (status !== 'granted') return null

    const projectId = Constants.expoConfig?.extra?.eas?.projectId
    if (!projectId) return null

    const { data } = await Notifications.getExpoPushTokenAsync({ projectId })
    return data
  } catch {
    return null
  }
}
