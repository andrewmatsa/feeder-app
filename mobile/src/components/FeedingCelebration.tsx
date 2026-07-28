import { Modal, View, StyleSheet } from 'react-native'
import { AquaMascot } from './AquaMascot'
import { AquariumBackground } from './AquariumBackground'
import { FeedingPellets } from './FeedingPellets'

const FISH_SIZE = 220
const FISH_HEIGHT = (FISH_SIZE * 140) / 200
// AquaMascot's mouth sits at roughly (36,78) in its own 200x140 viewBox —
// converted to a pixel position within the fish's rendered box (which is
// also the pellets' overlay container, via absoluteFill + alignItems/
// justifyContent: center both being the same box), so pellets can converge
// on the actual mouth instead of a generic spot.
const MOUTH_X = (36 / 200) * FISH_SIZE
const MOUTH_Y = (78 / 140) * FISH_HEIGHT

// Full-screen "the fish is eating" moment — same ocean/seaweed backdrop as
// the login page, shown for a few seconds right after a manual feed.
export function FeedingCelebration({ visible }: { visible: boolean }) {
  return (
    <Modal visible={visible} transparent animationType="fade" statusBarTranslucent>
      <View style={styles.screen}>
        <AquariumBackground />
        <View style={styles.scene}>
          <AquaMascot size={FISH_SIZE} eating={visible} />
          <FeedingPellets active={visible} targetX={MOUTH_X} targetY={MOUTH_Y} />
        </View>
      </View>
    </Modal>
  )
}

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: '#05222f', alignItems: 'center', justifyContent: 'center' },
  scene: { alignItems: 'center', justifyContent: 'center' },
})
