import { StyleSheet, Text, View } from 'react-native'
import { useSafeAreaInsets } from 'react-native-safe-area-context'
import { FishLogo } from './FishLogo'

// Persistent brand header shown on every screen — mirrors frontend's
// Layout.tsx .header-brand (small 44px fish + "AquaFeed" + page subtitle),
// which wraps every route on the web, not just Login.
export function AppHeader({ subtitle, right }: { subtitle?: string; right?: React.ReactNode }) {
  const insets = useSafeAreaInsets()
  return (
    <View style={[styles.container, { paddingTop: insets.top + 8 }]}>
      <View style={styles.brand}>
        <FishLogo size={36} />
        <View style={styles.textCol}>
          <Text style={styles.title}>AquaFeed</Text>
          {subtitle && <Text style={styles.subtitle}>{subtitle}</Text>}
        </View>
      </View>
      {right && <View>{right}</View>}
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 16,
    paddingBottom: 8,
    borderBottomWidth: 1,
    borderBottomColor: '#eee',
    backgroundColor: '#fff',
  },
  brand: { flexDirection: 'row', alignItems: 'center', gap: 8 },
  textCol: { justifyContent: 'center' },
  title: { fontSize: 16, fontWeight: '700', color: '#1f2937' },
  subtitle: { fontSize: 11, color: '#9ca3af' },
})
