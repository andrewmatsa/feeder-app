import { useEffect, useRef } from 'react'
import { Animated, Easing } from 'react-native'
import Svg, { Circle, ClipPath, Defs, Ellipse, G, LinearGradient, Path, RadialGradient, Stop } from 'react-native-svg'

const AnimatedG = Animated.createAnimatedComponent(G)
const AnimatedEllipse = Animated.createAnimatedComponent(Ellipse)

// Small header-icon version of AquaMascot.tsx — same fish, same visual
// layers (fins, clipped highlight bands, mouth bubbles), just idle-only
// (no tap/dart/eye-widen, since this one isn't interactive) and sized for
// inline use next to a title. Rotation/skew go through SVG `transform`
// strings on wrapping <G>s, not `rotation`/`origin` props directly — the
// latter aren't reliably interpolated by react-native-svg.
export function FishLogo({ size = 78 }: { size?: number }) {
  const bob = useRef(new Animated.Value(0)).current
  const tail = useRef(new Animated.Value(0)).current
  const fin = useRef(new Animated.Value(0)).current
  const pelvic = useRef(new Animated.Value(0)).current
  const dorsal = useRef(new Animated.Value(0)).current
  const blink = useRef(new Animated.Value(0)).current
  const mouthBubbles = [useRef(new Animated.Value(0)).current, useRef(new Animated.Value(0)).current, useRef(new Animated.Value(0)).current]

  useEffect(() => {
    const cssEase = Easing.bezier(0.42, 0, 0.58, 1)
    // bob drives the outer Animated.View's real transform (native driver
    // OK); everything else drives react-native-svg props directly, so it
    // has to stay JS-driven or it silently doesn't animate correctly.
    const swing = (v: Animated.Value, halfDuration: number, delay = 0, native = false) =>
      Animated.loop(
        Animated.sequence([
          ...(delay ? [Animated.delay(delay)] : []),
          Animated.timing(v, { toValue: 1, duration: halfDuration, easing: cssEase, useNativeDriver: native }),
          Animated.timing(v, { toValue: 0, duration: halfDuration, easing: cssEase, useNativeDriver: native }),
        ]),
      )
    const anims = [
      swing(bob, 2100, 0, true), // af-bob: 4.2s ease-in-out
      swing(tail, 425), // af-tail: 0.85s ease-in-out
      swing(fin, 700), // af-fin: 1.4s ease-in-out
      swing(pelvic, 700, 350), // af-pelvic: 1.4s ease-in-out, -0.35s offset
      swing(dorsal, 950), // af-dorsal: 1.9s ease-in-out
      Animated.loop(Animated.timing(blink, { toValue: 1, duration: 4600, easing: Easing.linear, useNativeDriver: false })),
      ...mouthBubbles.map((v, i) =>
        Animated.loop(
          Animated.sequence([
            Animated.delay(i * 900),
            Animated.timing(v, { toValue: 1, duration: 2600, easing: Easing.out(Easing.quad), useNativeDriver: false }),
            Animated.timing(v, { toValue: 0, duration: 0, useNativeDriver: false }),
            Animated.delay(1800 - i * 300),
          ]),
        ),
      ),
    ]
    anims.forEach((a) => a.start())
    return () => anims.forEach((a) => a.stop())
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const bobY = bob.interpolate({ inputRange: [0, 1], outputRange: [0, -3] })
  const bobRotate = bob.interpolate({ inputRange: [0, 1], outputRange: ['-0.6deg', '0.9deg'] })
  const tailTransform = tail.interpolate({ inputRange: [0, 1], outputRange: ['rotate(-15 149 70)', 'rotate(13 149 70)'] })
  const finTransform = fin.interpolate({ inputRange: [0, 1], outputRange: ['rotate(-7 94 92)', 'rotate(12 94 92)'] })
  const pelvicTransform = pelvic.interpolate({ inputRange: [0, 1], outputRange: ['rotate(8 123 103)', 'rotate(-9 123 103)'] })
  const dorsalTransform = dorsal.interpolate({ inputRange: [0, 1], outputRange: ['skewX(-4)', 'skewX(5)'] })
  const blinkRy = blink.interpolate({ inputRange: [0, 0.965, 0.98, 1], outputRange: [0, 0, 14, 0] })
  const bubbleY = (v: Animated.Value, base: number) => v.interpolate({ inputRange: [0, 1], outputRange: [base, base - 34] })
  const bubbleX = (v: Animated.Value, base: number) => v.interpolate({ inputRange: [0, 1], outputRange: [base, base - 10] })
  const bubbleOpacity = (v: Animated.Value) => v.interpolate({ inputRange: [0, 0.15, 1], outputRange: [0, 0.9, 0] })

  return (
    <Animated.View style={{ transform: [{ translateY: bobY }, { rotate: bobRotate }] }}>
      <Svg width={size} height={(size * 140) / 200} viewBox="0 0 200 140">
        <Defs>
          <RadialGradient id="afVolSm" cx="34%" cy="24%" r="82%">
            <Stop offset="0%" stopColor="#ffe0b0" />
            <Stop offset="30%" stopColor="#ffab5e" />
            <Stop offset="66%" stopColor="#f0682a" />
            <Stop offset="100%" stopColor="#a83b0e" />
          </RadialGradient>
          <LinearGradient id="afFinSm" x1="0" y1="0" x2="1" y2="0">
            <Stop offset="0%" stopColor="#ff8a3d" stopOpacity={0.95} />
            <Stop offset="60%" stopColor="#ff9f5e" stopOpacity={0.72} />
            <Stop offset="100%" stopColor="#ffd7a8" stopOpacity={0.42} />
          </LinearGradient>
          <LinearGradient id="afFinUpSm" x1="0" y1="1" x2="0" y2="0">
            <Stop offset="0%" stopColor="#f9803a" stopOpacity={0.92} />
            <Stop offset="100%" stopColor="#ffd6a6" stopOpacity={0.45} />
          </LinearGradient>
          <RadialGradient id="afEyeBallSm" cx="34%" cy="28%" r="80%">
            <Stop offset="0%" stopColor="#ffffff" />
            <Stop offset="70%" stopColor="#f6ead6" />
            <Stop offset="100%" stopColor="#cfa887" />
          </RadialGradient>
          <RadialGradient id="afIrisSm" cx="34%" cy="26%" r="85%">
            <Stop offset="0%" stopColor="#4a5f6b" />
            <Stop offset="55%" stopColor="#1d2c35" />
            <Stop offset="100%" stopColor="#070d11" />
          </RadialGradient>
          <ClipPath id="afClipSm">
            <Path d="M26,70 C38,40 64,24 96,24 C126,24 148,42 154,70 C148,98 126,116 96,116 C64,116 38,100 26,70 Z" />
          </ClipPath>
        </Defs>

        {/* tail */}
        <AnimatedG transform={tailTransform}>
          <Path
            d="M149,70 C163,54 181,36 196,27 C201,41 193,56 189,70 C193,84 201,99 196,113 C181,104 163,86 149,70 Z"
            fill="url(#afFinSm)"
          />
        </AnimatedG>

        {/* pectoral fin */}
        <AnimatedG transform={finTransform}>
          <Path d="M92,90 C79,100 74,120 88,128 C102,122 104,102 100,90 Z" fill="url(#afFinUpSm)" />
        </AnimatedG>

        {/* pelvic fin */}
        <AnimatedG transform={pelvicTransform}>
          <Path d="M120,102 C114,112 116,124 127,126 C132,116 130,106 126,100 Z" fill="url(#afFinUpSm)" opacity={0.85} />
        </AnimatedG>

        {/* dorsal fin */}
        <AnimatedG transform={dorsalTransform}>
          <Path d="M76,32 C86,6 118,1 136,27 C118,19 94,21 76,32 Z" fill="url(#afFinUpSm)" />
        </AnimatedG>

        {/* small trailing fin */}
        <AnimatedG transform={finTransform}>
          <Path d="M104,84 C98,96 100,114 112,120 C120,110 120,94 116,84 Z" fill="#a8420f" opacity={0.45} />
        </AnimatedG>

        {/* body */}
        <Path
          d="M26,70 C38,40 64,24 96,24 C126,24 148,42 154,70 C148,98 126,116 96,116 C64,116 38,100 26,70 Z"
          fill="url(#afVolSm)"
        />

        {/* highlight bands, clipped to the body silhouette */}
        <G clipPath="url(#afClipSm)">
          <G fill="#fff6e6" opacity={0.82}>
            <Path d="M56,20 C64,46 64,94 55,120 L72,122 C81,94 81,44 73,18 Z" />
            <Path d="M100,20 C108,46 108,94 99,120 L113,122 C122,94 122,44 114,18 Z" />
            <Path d="M136,26 C143,48 143,90 135,114 L145,110 C152,90 152,48 145,28 Z" opacity={0.7} />
          </G>
        </G>

        {/* mouth */}
        <Path d="M28,76 C33,82 41,82 45,77" fill="none" stroke="#8c2f14" strokeWidth={3.2} strokeLinecap="round" />

        {mouthBubbles.map((v, i) => {
          const bases: [number, number][] = [[22, 62], [17, 52], [13, 42]]
          const base = bases[i]
          const r = [3, 2.2, 4][i]
          return (
            <AnimatedEllipse
              key={i}
              cx={bubbleX(v, base[0])}
              cy={bubbleY(v, base[1])}
              rx={r}
              ry={r}
              fill="#eafcff"
              opacity={bubbleOpacity(v)}
            />
          )
        })}

        {/* eye */}
        <Circle cx={56} cy={60} r={14.5} fill="url(#afEyeBallSm)" />
        <Circle cx={56} cy={60} r={14.5} fill="none" stroke="#b34a12" strokeWidth={1.8} opacity={0.4} />
        <Circle cx={59} cy={61} r={7.8} fill="url(#afIrisSm)" />
        <Circle cx={62.2} cy={57} r={2.9} fill="#fff" />
        <Circle cx={55.5} cy={65.5} r={1.5} fill="#fff" opacity={0.55} />

        {/* eyelid (blinks) */}
        <AnimatedEllipse cx={56} cy={60} rx={15.2} ry={blinkRy} fill="#f2702d" />
      </Svg>
    </Animated.View>
  )
}
