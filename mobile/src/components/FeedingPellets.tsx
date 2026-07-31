import { useEffect, useRef } from 'react'
import { Animated, Easing, StyleSheet, View } from 'react-native'

const PELLETS = [
  { startX: -46, startY: -70, size: 9, delay: 0 },
  { startX: -8, startY: -92, size: 11, delay: 90 },
  { startX: 22, startY: -60, size: 8, delay: 220 },
  { startX: -30, startY: -110, size: 12, delay: 60 },
  { startX: 10, startY: -78, size: 9, delay: 160 },
]

// Ported from the "AquaFeed Fish" design artifact's af-pellet keyframe, but
// re-aimed: pellets converge on a single target point (the fish's mouth,
// passed in by the caller) and shrink away there — "eaten" — instead of
// just drifting past in a generic column.
export function FeedingPellets({
  active,
  targetX = 0,
  targetY = 0,
}: {
  active: boolean
  targetX?: number
  targetY?: number
}) {
  const values = useRef(PELLETS.map(() => new Animated.Value(0))).current

  useEffect(() => {
    if (!active) return
    values.forEach((v) => v.setValue(0))
    const anims = values.map((v, i) =>
      Animated.timing(v, {
        toValue: 1,
        duration: 620,
        delay: PELLETS[i].delay,
        easing: Easing.in(Easing.cubic),
        useNativeDriver: true,
      }),
    )
    Animated.parallel(anims).start()
  }, [active, values])

  if (!active) return null

  return (
    <View style={StyleSheet.absoluteFill} pointerEvents="none">
      {PELLETS.map((p, i) => {
        const translateX = values[i].interpolate({ inputRange: [0, 1], outputRange: [p.startX, 0] })
        const translateY = values[i].interpolate({ inputRange: [0, 1], outputRange: [p.startY, 0] })
        // Visible for the fall, then shrinks/fades right at arrival — "eaten".
        const scale = values[i].interpolate({ inputRange: [0, 0.75, 0.92, 1], outputRange: [1, 1, 0.4, 0] })
        const opacity = values[i].interpolate({ inputRange: [0, 0.1, 0.85, 1], outputRange: [0, 1, 1, 0] })
        return (
          <Animated.View
            key={i}
            style={[
              styles.pellet,
              {
                left: targetX - p.size / 2,
                top: targetY - p.size / 2,
                width: p.size,
                height: p.size,
                borderRadius: p.size / 2,
                opacity,
                transform: [{ translateX }, { translateY }, { scale }],
              },
            ]}
          />
        )
      })}
    </View>
  )
}

const styles = StyleSheet.create({
  pellet: { position: 'absolute', backgroundColor: '#c1762c', shadowColor: '#000', shadowOpacity: 0.3, shadowRadius: 2, shadowOffset: { width: 0, height: 1 } },
})
