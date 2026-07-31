const versions = require('../versions.json')

// Single source of truth for version strings across firmware/backend/frontend/mobile — see CLAUDE.md.
module.exports = {
  expo: {
    name: 'AquaFeed',
    slug: 'aquafeed-mobile',
    scheme: 'aquafeed',
    version: versions.appVersion,
    orientation: 'portrait',
    icon: './assets/icon.png',
    userInterfaceStyle: 'light',
    ios: {
      supportsTablet: true,
      bundleIdentifier: 'com.aquafeed.mobile',
      buildNumber: versions.appVersion,
    },
    android: {
      package: 'com.aquafeed.mobile',
      versionCode: 1,
      adaptiveIcon: {
        backgroundColor: '#E6F4FE',
        foregroundImage: './assets/android-icon-foreground.png',
        backgroundImage: './assets/android-icon-background.png',
        monochromeImage: './assets/android-icon-monochrome.png',
      },
      predictiveBackGestureEnabled: false,
    },
    web: {
      favicon: './assets/favicon.png',
    },
    plugins: [
      'expo-router',
      'expo-secure-store',
      'expo-notifications',
      [
        'expo-splash-screen',
        {
          image: './assets/splash-icon.png',
          imageWidth: 200,
          resizeMode: 'contain',
          backgroundColor: '#E6F4FE',
        },
      ],
    ],
    // extra.eas.projectId is required for push tokens (Notifications.getExpoPushTokenAsync)
    // and is not set yet — run `eas init` (see mobile/eas.json) to generate one, then add it
    // here. Until then, registerForPushNotificationsAsync() safely no-ops.
  },
}
