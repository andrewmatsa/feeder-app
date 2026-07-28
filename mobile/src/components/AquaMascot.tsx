import { useEffect, useRef, useState } from 'react'
import { Animated, Dimensions, Easing, Pressable } from 'react-native'
import Svg, { Circle, ClipPath, Defs, Ellipse, G, LinearGradient, Path, RadialGradient, Stop } from 'react-native-svg'

const AnimatedG = Animated.createAnimatedComponent(G)
const AnimatedCircle = Animated.createAnimatedComponent(Circle)
const AnimatedEllipse = Animated.createAnimatedComponent(Ellipse)
const AnimatedPath = Animated.createAnimatedComponent(Path)

// Ported from the "AquaFeed Fish" design artifact (rich orange/gold gradient
// body, layered fins, detailed eye) — visual + idle animation only; the
// artifact's wander/feed/scare AI stays web-only by choice, this keeps the
// existing tap-to-dart interaction instead.
// Rotation/skew go through SVG `transform` strings on wrapping <G>s rather
// than `rotation`/`origin` props directly — the latter aren't reliably
// interpolated by react-native-svg (confirmed while debugging earlier fish
// and seaweed passes where the same shortcut left things looking frozen).
export function AquaMascot({ size = 190, eating = false }: { size?: number; eating?: boolean }) {
  const bob = useRef(new Animated.Value(0)).current
  const tail = useRef(new Animated.Value(0)).current
  const fin = useRef(new Animated.Value(0)).current
  const pelvic = useRef(new Animated.Value(0)).current
  const dorsal = useRef(new Animated.Value(0)).current
  const blink = useRef(new Animated.Value(0)).current
  const mouthBubbles = [useRef(new Animated.Value(0)).current, useRef(new Animated.Value(0)).current, useRef(new Animated.Value(0)).current]
  const escape = useRef(new Animated.Value(0)).current
  const burst = useRef(new Animated.Value(0)).current
  const eyeWiden = useRef(new Animated.Value(0)).current
  const chomp = useRef(new Animated.Value(0)).current
  const escaping = useRef(false)
  const [isBursting, setIsBursting] = useState(false)

  // Chomping: while `eating`, open/close the mouth on a fast repeating
  // loop; snap fully closed the moment feeding stops.
  useEffect(() => {
    if (!eating) {
      chomp.stopAnimation()
      chomp.setValue(0)
      return
    }
    const loop = Animated.loop(
      Animated.sequence([
        Animated.timing(chomp, { toValue: 1, duration: 160, easing: Easing.out(Easing.quad), useNativeDriver: false }),
        Animated.timing(chomp, { toValue: 0, duration: 200, easing: Easing.in(Easing.quad), useNativeDriver: false }),
        Animated.delay(80),
      ]),
    )
    loop.start()
    return () => loop.stop()
  }, [eating, chomp])

  useEffect(() => {
    // CSS's `ease-in-out` timing function, matched exactly (not RN's
    // sine/quad approximations) — this is what actually makes the swing
    // read as smooth acceleration/deceleration instead of a mechanical
    // linear sweep with a kink at the midpoint.
    const cssEase = Easing.bezier(0.42, 0, 0.58, 1)
    // native driver only understands RN's own View `transform`/`opacity` —
    // not arbitrary SVG `transform` strings or shape attributes (cx/cy/ry).
    // bob/dart animate the outer Animated.View's real transform (native OK);
    // everything else here drives react-native-svg props directly, so it
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

  // Tap: a real startle, not an instant dash — the tail winds up and the
  // eyes go wide FIRST while the fish stays put, then it accelerates off
  // the left edge of the screen (slow to fast, not full-speed immediately),
  // wraps to the right edge, and glides calmly back to center.
  const handlePress = () => {
    if (escaping.current) return
    escaping.current = true
    const screenW = Dimensions.get('window').width
    const travel = screenW / 2 + size
    const windupDuration = 320
    const fleeDuration = 900
    const returnDuration = 950

    // eyes widen right away, then relax over the windup+flee window
    eyeWiden.setValue(0)
    Animated.sequence([
      Animated.timing(eyeWiden, { toValue: 1, duration: 140, easing: Easing.out(Easing.ease), useNativeDriver: false }),
      Animated.delay(windupDuration),
      Animated.timing(eyeWiden, { toValue: 0, duration: 500, easing: Easing.inOut(Easing.ease), useNativeDriver: false }),
    ]).start()

    // tail starts wagging fast immediately (the "windup"); the body itself
    // doesn't move until that's had a moment to read, then it eases from a
    // standstill up to full flee speed rather than jumping straight to it.
    setIsBursting(true)
    burst.setValue(0)
    Animated.loop(
      Animated.timing(burst, { toValue: 1, duration: 110, easing: Easing.inOut(Easing.ease), useNativeDriver: false }),
      { iterations: Math.ceil((windupDuration + fleeDuration) / 110) },
    ).start(() => {
      burst.setValue(0)
      setIsBursting(false)
    })

    escape.setValue(0)
    Animated.sequence([
      Animated.delay(windupDuration),
      Animated.timing(escape, {
        toValue: -travel,
        duration: fleeDuration,
        easing: Easing.in(Easing.quad),
        useNativeDriver: true,
      }),
    ]).start(() => {
      escape.setValue(travel)
      Animated.timing(escape, {
        toValue: 0,
        duration: returnDuration,
        easing: Easing.out(Easing.cubic),
        useNativeDriver: true,
      }).start(() => {
        escaping.current = false
      })
    })
  }

  const bobY = bob.interpolate({ inputRange: [0, 1], outputRange: [0, -14] })
  const bobRotate = bob.interpolate({ inputRange: [0, 1], outputRange: ['-2deg', '2deg'] })
  const dartX = escape

  const tailTransform = tail.interpolate({ inputRange: [0, 1], outputRange: ['rotate(-15 149 70)', 'rotate(13 149 70)'] })
  const burstTransform = burst.interpolate({
    inputRange: [0, 0.5, 1],
    outputRange: ['rotate(0 149 70)', 'rotate(-22 149 70)', 'rotate(18 149 70)'],
  })
  const finTransform = fin.interpolate({ inputRange: [0, 1], outputRange: ['rotate(-7 94 92)', 'rotate(12 94 92)'] })
  const pelvicTransform = pelvic.interpolate({ inputRange: [0, 1], outputRange: ['rotate(8 123 103)', 'rotate(-9 123 103)'] })
  const dorsalTransform = dorsal.interpolate({ inputRange: [0, 1], outputRange: ['skewX(-4)', 'skewX(5)'] })
  const blinkRy = blink.interpolate({
    inputRange: [0, 0.965, 0.98, 1],
    outputRange: [0, 0, 14, 0],
  })
  const eyeScaleTransform = eyeWiden.interpolate({
    inputRange: [0, 1],
    outputRange: ['translate(56 60) scale(1) translate(-56 -60)', 'translate(56 60) scale(1.45) translate(-56 -60)'],
  })

  const bubbleY = (v: Animated.Value, base: number) => v.interpolate({ inputRange: [0, 1], outputRange: [base, base - 34] })
  const bubbleX = (v: Animated.Value, base: number) => v.interpolate({ inputRange: [0, 1], outputRange: [base, base - 10] })
  const bubbleOpacity = (v: Animated.Value) => v.interpolate({ inputRange: [0, 0.15, 1], outputRange: [0, 0.9, 0] })

  return (
    <Pressable onPress={handlePress} accessibilityRole="button" accessibilityLabel="Стилізована рибка AquaFeed">
      <Animated.View style={{ transform: [{ translateY: bobY }, { rotate: bobRotate }, { translateX: dartX }] }}>
        <Svg width={size} height={(size * 140) / 200} viewBox="0 0 200 140">
          <Defs>
            <RadialGradient id="afVol" cx="34%" cy="24%" r="82%">
              <Stop offset="0%" stopColor="#ffe0b0" />
              <Stop offset="30%" stopColor="#ffab5e" />
              <Stop offset="66%" stopColor="#f0682a" />
              <Stop offset="100%" stopColor="#a83b0e" />
            </RadialGradient>
            <LinearGradient id="afFin" x1="0" y1="0" x2="1" y2="0">
              <Stop offset="0%" stopColor="#ff8a3d" stopOpacity={0.95} />
              <Stop offset="60%" stopColor="#ff9f5e" stopOpacity={0.72} />
              <Stop offset="100%" stopColor="#ffd7a8" stopOpacity={0.42} />
            </LinearGradient>
            <LinearGradient id="afFinUp" x1="0" y1="1" x2="0" y2="0">
              <Stop offset="0%" stopColor="#f9803a" stopOpacity={0.92} />
              <Stop offset="100%" stopColor="#ffd6a6" stopOpacity={0.45} />
            </LinearGradient>
            <RadialGradient id="afEyeBall" cx="34%" cy="28%" r="80%">
              <Stop offset="0%" stopColor="#ffffff" />
              <Stop offset="70%" stopColor="#f6ead6" />
              <Stop offset="100%" stopColor="#cfa887" />
            </RadialGradient>
            <RadialGradient id="afIris" cx="34%" cy="26%" r="85%">
              <Stop offset="0%" stopColor="#4a5f6b" />
              <Stop offset="55%" stopColor="#1d2c35" />
              <Stop offset="100%" stopColor="#070d11" />
            </RadialGradient>
            <ClipPath id="afClip">
              <Path d="M26,70 C38,40 64,24 96,24 C126,24 148,42 154,70 C148,98 126,116 96,116 C64,116 38,100 26,70 Z" />
            </ClipPath>
          </Defs>

          {/* tail */}
          <AnimatedG transform={isBursting ? burstTransform : tailTransform}>
            <Path
              d="M149,70 C163,54 181,36 196,27 C201,41 193,56 189,70 C193,84 201,99 196,113 C181,104 163,86 149,70 Z"
              fill="url(#afFin)"
            />
          </AnimatedG>

          {/* pectoral fin */}
          <AnimatedG transform={finTransform}>
            <Path d="M92,90 C79,100 74,120 88,128 C102,122 104,102 100,90 Z" fill="url(#afFinUp)" />
          </AnimatedG>

          {/* pelvic fin */}
          <AnimatedG transform={pelvicTransform}>
            <Path d="M120,102 C114,112 116,124 127,126 C132,116 130,106 126,100 Z" fill="url(#afFinUp)" opacity={0.85} />
          </AnimatedG>

          {/* dorsal fin */}
          <AnimatedG transform={dorsalTransform}>
            <Path d="M76,32 C86,6 118,1 136,27 C118,19 94,21 76,32 Z" fill="url(#afFinUp)" />
          </AnimatedG>

          {/* small trailing fin */}
          <AnimatedG transform={finTransform}>
            <Path d="M104,84 C98,96 100,114 112,120 C120,110 120,94 116,84 Z" fill="#a8420f" opacity={0.45} />
          </AnimatedG>

          {/* body */}
          <Path
            d="M26,70 C38,40 64,24 96,24 C126,24 148,42 154,70 C148,98 126,116 96,116 C64,116 38,100 26,70 Z"
            fill="url(#afVol)"
          />

          {/* highlight bands, clipped to the body silhouette */}
          <G clipPath="url(#afClip)">
            <G fill="#fff6e6" opacity={0.82}>
              <Path d="M56,20 C64,46 64,94 55,120 L72,122 C81,94 81,44 73,18 Z" />
              <Path d="M100,20 C108,46 108,94 99,120 L113,122 C122,94 122,44 114,18 Z" />
              <Path d="M136,26 C143,48 143,90 135,114 L145,110 C152,90 152,48 145,28 Z" opacity={0.7} />
            </G>
          </G>

          {/* mouth — closed curve fades out / open oval fades in while eating */}
          <AnimatedPath
            d="M28,76 C33,82 41,82 45,77"
            fill="none"
            stroke="#8c2f14"
            strokeWidth={3.2}
            strokeLinecap="round"
            opacity={chomp.interpolate({ inputRange: [0, 1], outputRange: [1, 0] })}
          />
          <AnimatedEllipse
            cx={36}
            cy={78}
            rx={9}
            ry={chomp.interpolate({ inputRange: [0, 1], outputRange: [0, 8] })}
            fill="#5a1c0c"
            opacity={chomp}
          />

          {mouthBubbles.map((v, i) => {
            const bases: [number, number][] = [[22, 62], [17, 52], [13, 42]]
            const base = bases[i]
            const r = [3, 2.2, 4][i]
            return (
              <AnimatedCircle
                key={i}
                cx={bubbleX(v, base[0])}
                cy={bubbleY(v, base[1])}
                r={r}
                fill="#eafcff"
                opacity={bubbleOpacity(v)}
              />
            )
          })}

          {/* eye — widens on tap (startle), via a wrapping <G> transform
              since scaling the shape attributes directly would need every
              radius/center recomputed by hand. */}
          <AnimatedG transform={eyeScaleTransform}>
            <Circle cx={56} cy={60} r={14.5} fill="url(#afEyeBall)" />
            <Circle cx={56} cy={60} r={14.5} fill="none" stroke="#b34a12" strokeWidth={1.8} opacity={0.4} />
            <Circle cx={59} cy={61} r={7.8} fill="url(#afIris)" />
            <Circle cx={62.2} cy={57} r={2.9} fill="#fff" />
            <Circle cx={55.5} cy={65.5} r={1.5} fill="#fff" opacity={0.55} />

            {/* eyelid (blinks) */}
            <AnimatedEllipse cx={56} cy={60} rx={15.2} ry={blinkRy} fill="#f2702d" />
          </AnimatedG>
        </Svg>
      </Animated.View>
    </Pressable>
  )
}
