import { useEffect, useRef, useState, type ReactNode } from 'react'
import { Animated, Easing, StyleSheet, View, type GestureResponderEvent } from 'react-native'
import { AquariumBackground } from './AquariumBackground'

interface Burst {
  id: number
  x: number
  y: number
}

const BUBBLE_OFFSETS = [
  { dx: -10, dy: -46, size: 9, delay: 0 },
  { dx: 6, dy: -66, size: 6, delay: 60 },
  { dx: -18, dy: -30, size: 5, delay: 120 },
  { dx: 14, dy: -40, size: 7, delay: 30 },
]

function TapBurst({ x, y, onDone }: { x: number; y: number; onDone: () => void }) {
  const v = useRef(new Animated.Value(0)).current
  useEffect(() => {
    Animated.timing(v, { toValue: 1, duration: 950, easing: Easing.out(Easing.quad), useNativeDriver: true }).start(onDone)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  return (
    <>
      {BUBBLE_OFFSETS.map((b, i) => {
        const translateY = v.interpolate({ inputRange: [0, 1], outputRange: [0, b.dy] })
        const translateX = v.interpolate({ inputRange: [0, 1], outputRange: [0, b.dx] })
        const opacity = v.interpolate({ inputRange: [0, 0.15, 1], outputRange: [0, 0.9, 0] })
        const scale = v.interpolate({ inputRange: [0, 0.2, 1], outputRange: [0.3, 1, 1.15] })
        return (
          <Animated.View
            key={i}
            pointerEvents="none"
            style={{
              position: 'absolute',
              left: x - b.size / 2,
              top: y - b.size / 2,
              width: b.size,
              height: b.size,
              borderRadius: b.size / 2,
              backgroundColor: 'rgba(255,255,255,0.8)',
              opacity,
              transform: [{ translateX }, { translateY }, { scale }],
            }}
          />
        )
      })}
    </>
  )
}

// Shared page shell — puts the same ocean gradient/bubbles/seaweed behind
// every screen in the app (not just login), so cards/lists read as pale
// content floating in the aquarium instead of on a flat white background.
// With `interactive`, tapping anywhere also pops a little burst of bubbles
// at the touch point — onTouchStart is a raw touch handler (not part of the
// gesture-responder system), so it fires alongside nested Pressables/lists
// without stealing their taps.
export function AquariumScreen({ children, interactive = false }: { children: ReactNode; interactive?: boolean }) {
  const [bursts, setBursts] = useState<Burst[]>([])
  const nextId = useRef(0)

  const handleTouchStart = (e: GestureResponderEvent) => {
    if (!interactive) return
    const { locationX, locationY } = e.nativeEvent
    const id = nextId.current++
    setBursts((prev) => [...prev, { id, x: locationX, y: locationY }])
  }

  return (
    <View style={styles.screen} onTouchStart={handleTouchStart}>
      <AquariumBackground />
      {interactive && (
        <View style={StyleSheet.absoluteFill} pointerEvents="none">
          {bursts.map((b) => (
            <TapBurst key={b.id} x={b.x} y={b.y} onDone={() => setBursts((prev) => prev.filter((x) => x.id !== b.id))} />
          ))}
        </View>
      )}
      {children}
    </View>
  )
}

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: '#05222f' },
})
