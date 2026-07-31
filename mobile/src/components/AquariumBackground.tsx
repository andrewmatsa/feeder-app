import { useEffect, useMemo, useRef } from 'react'
import { Animated, Dimensions, type DimensionValue, Easing, StyleSheet, View } from 'react-native'
import Svg, { Defs, LinearGradient, Path, Rect, Stop } from 'react-native-svg'

const AnimatedPath = Animated.createAnimatedComponent(Path)

const VIEW_H = 220
const RENDER_H = 250

interface PlantSpec {
  dLeft: string
  dRight: string
  gradId: string
  duration: number
  delay: number
}

// Ribbon kelp: a tall tapered blade. Bending is done by morphing the path
// itself between two lean states (base anchored, tip and mid-curve offset
// sideways) rather than rigidly rotating the whole shape around its root —
// a rigid rotation reads as a stiff plank pivoting, not a blade flexing.
function ribbonPath(x: number, height: number, width: number, curve: number): string {
  const base = VIEW_H
  const tipY = base - height
  const tipX = x + curve
  const midY1 = base - height * 0.35
  const midY2 = base - height * 0.75
  return (
    `M${x - width / 2},${base} ` +
    `C${x - width / 2 + curve * 0.15},${midY1} ${x - width * 0.15 + curve * 0.6},${midY2} ${tipX},${tipY} ` +
    `C${x + width * 0.15 + curve * 0.6},${midY2} ${x + width / 2 + curve * 0.15},${midY1} ${x + width / 2},${base} Z`
  )
}

// Rounded-tip leaf (Amazon-sword style) — fuller/wider than the ribbon kelp,
// with a blunt rather than pointed tip.
function leafPath(x: number, height: number, width: number, curve: number): string {
  const base = VIEW_H
  const tipY = base - height
  const tipX = x + curve
  const midY = base - height * 0.55
  const w2 = width / 2
  return (
    `M${x - w2 * 0.3},${base} ` +
    `C${x - w2 + curve * 0.1},${base - height * 0.25} ${x - w2 * 0.9 + curve * 0.45},${midY} ${tipX - width * 0.12},${tipY + height * 0.08} ` +
    `C${tipX - width * 0.05},${tipY} ${tipX + width * 0.05},${tipY} ${tipX + width * 0.12},${tipY + height * 0.08} ` +
    `C${x + w2 * 0.9 + curve * 0.45},${midY} ${x + w2 - curve * 0.1},${base - height * 0.25} ${x + w2 * 0.3},${base} Z`
  )
}

// Thin grass blade — same ribbon silhouette, just slim.
function grassPath(x: number, height: number, width: number, curve: number): string {
  return ribbonPath(x, height, width, curve)
}

function kelpFronds(): PlantSpec[] {
  const specs = [
    { x: 22, height: 190, width: 26, bend: 20, hue: 'a', duration: 10400, delay: 600 },
    { x: 54, height: 150, width: 20, bend: 16, hue: 'b', duration: 8200, delay: 2100 },
    { x: 200, height: 168, width: 22, bend: 18, hue: 'a', duration: 12000, delay: 0 },
    { x: 344, height: 176, width: 24, bend: 19, hue: 'b', duration: 11200, delay: 3400 },
    { x: 380, height: 138, width: 19, bend: 15, hue: 'a', duration: 9200, delay: 1200 },
  ]
  return specs.map((s) => ({
    dLeft: ribbonPath(s.x, s.height, s.width, -s.bend),
    dRight: ribbonPath(s.x, s.height, s.width, s.bend),
    gradId: `kelp${s.hue}`,
    duration: s.duration,
    delay: s.delay,
  }))
}

function leafPlants(): PlantSpec[] {
  const specs = [
    { x: 118, height: 108, width: 30, bend: 14, hue: 'c', duration: 9600, delay: 1500 },
    { x: 282, height: 96, width: 26, bend: 12, hue: 'a', duration: 8600, delay: 900 },
    { x: 168, height: 84, width: 22, bend: 11, hue: 'b', duration: 7600, delay: 2600 },
  ]
  return specs.map((s) => ({
    dLeft: leafPath(s.x, s.height, s.width, -s.bend),
    dRight: leafPath(s.x, s.height, s.width, s.bend),
    gradId: `leaf${s.hue}`,
    duration: s.duration,
    delay: s.delay,
  }))
}

// Grass tufts tucked between the kelp/leaf plants — several slim blades per
// cluster, each a bit different so the tuft moves like real aquarium grass.
function reedCluster(clusterX: number, count: number, gradIds: string[], phase: number): PlantSpec[] {
  return Array.from({ length: count }, (_, i) => {
    const x = clusterX + (i - (count - 1) / 2) * 7
    const height = 58 + ((i * 17 + phase) % 34)
    const width = 6 + (i % 2) * 2
    const bend = 9 + (i % 3) * 2
    return {
      dLeft: grassPath(x, height, width, -bend),
      dRight: grassPath(x, height, width, bend),
      gradId: gradIds[i % gradIds.length],
      duration: 5200 + ((i * 340 + phase * 7) % 3200),
      delay: (i * 260 + phase * 11) % 1400,
    }
  })
}

const KELP = kelpFronds()
const LEAVES = leafPlants()
const REEDS = [
  ...reedCluster(88, 6, ['reedA', 'reedB'], 5),
  ...reedCluster(240, 5, ['reedB', 'reedC'], 40),
  ...reedCluster(360, 4, ['reedA', 'reedC'], 20),
]

// Bends by morphing the path `d` string between two lean states, rather than
// rotating a rigid shape around its base — react-native-svg interpolates the
// numeric coordinates directly since dLeft/dRight share the same command
// structure (only coordinates differ). This is also why an earlier attempt
// at rotating via `rotation`/`origin` props looked frozen: those aren't
// reliably interpolated, whereas plain path/attribute values are.
function Plant({ dLeft, dRight, gradId, duration, delay }: PlantSpec) {
  const v = useRef(new Animated.Value(0)).current
  useEffect(() => {
    const cssEase = Easing.bezier(0.42, 0, 0.58, 1)
    const anim = Animated.loop(
      Animated.sequence([
        ...(delay ? [Animated.delay(delay)] : []),
        Animated.timing(v, { toValue: 1, duration, easing: cssEase, useNativeDriver: false }),
        Animated.timing(v, { toValue: 0, duration, easing: cssEase, useNativeDriver: false }),
      ]),
    )
    anim.start()
    return () => anim.stop()
  }, [v, duration, delay])

  const d = v.interpolate({ inputRange: [0, 1], outputRange: [dLeft, dRight] })

  return <AnimatedPath d={d} fill={`url(#${gradId})`} />
}

function Bubble({ left, size, duration, delay, drift }: { left: string; size: number; duration: number; delay: number; drift: number }) {
  const v = useRef(new Animated.Value(0)).current
  useEffect(() => {
    // Stagger only the very first rise so bubbles don't all launch in sync —
    // the loop itself repeats just the rise, back-to-back, so each bubble
    // keeps rising continuously instead of vanishing then sitting idle for
    // another full `delay` before its next appearance.
    const rise = Animated.loop(
      Animated.timing(v, { toValue: 1, duration, easing: Easing.linear, useNativeDriver: true }),
    )
    const startTimer = setTimeout(() => {
      v.setValue(0)
      rise.start()
    }, delay)
    return () => {
      clearTimeout(startTimer)
      rise.stop()
    }
  }, [v, duration, delay])

  const screenH = Dimensions.get('window').height
  const translateY = v.interpolate({ inputRange: [0, 1], outputRange: [0, -screenH * 0.95] })
  const translateX = v.interpolate({ inputRange: [0, 1], outputRange: [0, drift] })
  const opacity = v.interpolate({ inputRange: [0, 0.1, 0.85, 1], outputRange: [0, 0.55, 0.35, 0] })

  return (
    <Animated.View
      style={{
        position: 'absolute',
        left: left as DimensionValue,
        bottom: -40,
        width: size,
        height: size,
        borderRadius: size / 2,
        backgroundColor: 'rgba(255,255,255,0.55)',
        transform: [{ translateY }, { translateX }],
        opacity,
      }}
    />
  )
}

// Ported from frontend/src/pages/LoginPage.css's .lp2-ocean-bg — gradient
// water, rising bubbles, swaying seaweed — retuned for RN (no CSS gradients
// on plain Views, so the gradient itself is an SVG Rect).
export function AquariumBackground() {
  // Deliberately irregular spacing/sizing/timing per bubble (not an evenly
  // spaced grid) so the field reads as scattered rather than a repeating
  // pattern of fixed lanes.
  const bubbles = useMemo(() => {
    const count = 18
    return Array.from({ length: count }, (_, i) => {
      const seed = (i * 47 + 13) % 97
      const duration = 11000 + ((i * 613 + seed * 31) % 5000)
      return {
        left: `${(seed * 3.7 + i * 11) % 96}%`,
        size: 5 + ((i * 23 + seed) % 12),
        duration,
        // Spread each bubble's launch across its own full cycle instead of a
        // shared 0-9000ms window, so arrivals are steady/staggered rather
        // than a cluster of pops firing at once every few seconds.
        delay: (i / count) * duration,
        drift: (i % 2 === 0 ? 1 : -1) * (14 + ((i * 17) % 22)),
      }
    })
  }, [])

  return (
    <View style={StyleSheet.absoluteFill} pointerEvents="none">
      <Svg width="100%" height="100%" style={StyleSheet.absoluteFill}>
        <Defs>
          <LinearGradient id="oceanGrad" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#1f8fb8" />
            <Stop offset="46%" stopColor="#0d4a63" />
            <Stop offset="100%" stopColor="#05222f" />
          </LinearGradient>
        </Defs>
        <Rect width="100%" height="100%" fill="url(#oceanGrad)" />
      </Svg>

      {bubbles.map((b, i) => (
        <Bubble key={i} {...b} />
      ))}

      <Svg
        width="100%"
        height={RENDER_H}
        style={{ position: 'absolute', left: 0, right: 0, bottom: 0 }}
        viewBox={`0 0 400 ${VIEW_H}`}
        preserveAspectRatio="none"
      >
        <Defs>
          <LinearGradient id="kelpa" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#3fd39a" />
            <Stop offset="55%" stopColor="#12a26a" />
            <Stop offset="100%" stopColor="#075c3d" />
          </LinearGradient>
          <LinearGradient id="kelpb" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#34c98f" />
            <Stop offset="55%" stopColor="#0f9463" />
            <Stop offset="100%" stopColor="#0a5a3d" />
          </LinearGradient>
          <LinearGradient id="leafa" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#7de8a8" />
            <Stop offset="60%" stopColor="#2fae74" />
            <Stop offset="100%" stopColor="#0f6b45" />
          </LinearGradient>
          <LinearGradient id="leafb" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#6fdca0" />
            <Stop offset="60%" stopColor="#22a06a" />
            <Stop offset="100%" stopColor="#0c5c3c" />
          </LinearGradient>
          <LinearGradient id="leafc" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#59d194" />
            <Stop offset="60%" stopColor="#189460" />
            <Stop offset="100%" stopColor="#0a5238" />
          </LinearGradient>
          <LinearGradient id="reedA" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#5eeab0" />
            <Stop offset="100%" stopColor="#12a26a" />
          </LinearGradient>
          <LinearGradient id="reedB" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#3fd39a" />
            <Stop offset="100%" stopColor="#0c8a5a" />
          </LinearGradient>
          <LinearGradient id="reedC" x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor="#28b884" />
            <Stop offset="100%" stopColor="#0a5a3d" />
          </LinearGradient>
        </Defs>
        {KELP.map((k, i) => (
          <Plant key={`k${i}`} {...k} />
        ))}
        {LEAVES.map((l, i) => (
          <Plant key={`l${i}`} {...l} />
        ))}
        {REEDS.map((r, i) => (
          <Plant key={`r${i}`} {...r} />
        ))}
      </Svg>
    </View>
  )
}
