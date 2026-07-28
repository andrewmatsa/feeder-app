import { useEffect, useRef } from 'react'
import { Animated, Easing } from 'react-native'
import Svg, { Ellipse, G, Path } from 'react-native-svg'

const AnimatedG = Animated.createAnimatedComponent(G)
const AnimatedPath = Animated.createAnimatedComponent(Path)
const AnimatedEllipse = Animated.createAnimatedComponent(Ellipse)

// Ported from frontend/src/components/LoginPage.tsx's idle SVG animation
// (hero-fish-swim / -tail-flick / -eye-blink / -bubble-rise keyframes) — the
// project's actual logo, just never wired into either app's active screens.
function loop(value: Animated.Value, toValue: number, duration: number, delay = 0) {
  return Animated.loop(
    Animated.sequence([
      Animated.timing(value, { toValue, duration, delay, easing: Easing.inOut(Easing.ease), useNativeDriver: true }),
      Animated.timing(value, { toValue: 0, duration, easing: Easing.inOut(Easing.ease), useNativeDriver: true }),
    ]),
  )
}

export function FishLogo({ size = 78 }: { size?: number }) {
  const sway = useRef(new Animated.Value(0)).current
  const tail = useRef(new Animated.Value(0)).current
  const blink = useRef(new Animated.Value(1)).current
  const bubble1 = useRef(new Animated.Value(0)).current
  const bubble2 = useRef(new Animated.Value(0)).current
  const bubble3 = useRef(new Animated.Value(0)).current

  useEffect(() => {
    const animations = [
      loop(sway, 1, 2100),
      loop(tail, 1, 420),
      Animated.loop(
        Animated.sequence([
          Animated.timing(blink, { toValue: 0.15, duration: 90, useNativeDriver: true }),
          Animated.timing(blink, { toValue: 1, duration: 90, useNativeDriver: true }),
          Animated.delay(5600),
        ]),
      ),
      loop(bubble1, 1, 1200, 0),
      loop(bubble2, 1, 1200, 400),
      loop(bubble3, 1, 1200, 800),
    ]
    animations.forEach((a) => a.start())
    return () => animations.forEach((a) => a.stop())
  }, [sway, tail, blink, bubble1, bubble2, bubble3])

  const swayY = sway.interpolate({ inputRange: [0, 1], outputRange: [0, -3] })
  const swayRotate = sway.interpolate({ inputRange: [0, 1], outputRange: ['-0.6deg', '0.9deg'] })
  const tailRotate = tail.interpolate({ inputRange: [0, 1], outputRange: ['0deg', '-5deg'] })
  const bubbleY = (v: Animated.Value, base: number) => v.interpolate({ inputRange: [0, 1], outputRange: [base, base - 6] })
  const bubbleOpacity = (v: Animated.Value) => v.interpolate({ inputRange: [0, 1], outputRange: [0.9, 0.55] })

  return (
    <Animated.View style={{ transform: [{ translateY: swayY }, { rotate: swayRotate }] }}>
      <Svg width={size} height={size} viewBox="0 0 64 64">
        <Path
          d="M58.7 41.5c0-3.5 4.9-11.4 2.6-13.8c-2.5-2.6-8.3 8.5-11.2 8.5c-3.5 0-5.6-4.3-7.3-6.1c-1.4-1.4 2.6-7 .8-7.4c-7.5-1.8-8.5 2.6-12.6 1.5c-3.2-.8-6.5-1.3-9.7-1.3c-12 0-14.3 8.6-16.4 16.6C4.5 40.7 16.6 51 16.6 51s-9.2-5.2-9-4c1.5 6.6 7.7 10.8 14.7 12.4c2 .5 4.1.7 6.1.7c12.8 0 14.8-9.9 21.7-11.1c4.2-.7 8.7 7.4 11.1 4.9c2.6-2.6-2.5-8.3-2.5-12.4"
          fill="#728389"
        />
        <G fill="#8d9ba3">
          <AnimatedPath
            d="M48.1 60.5c-1.2 1.2-3.6 2.7-6.2 0s-5.4-7.5-4.2-8.7c1.2-1.2 5.8 1.7 8.4 4.4c2.6 2.6 3.2 3.1 2 4.3"
            rotation={tailRotate}
            origin="38,56"
          />
          <Ellipse cx={33.4} cy={35.3} rx={2.2} ry={3.2} />
          <Ellipse cx={37.6} cy={39.2} rx={1.2} ry={2.5} />
          <Ellipse cx={39.9} cy={36} rx={0.6} ry={1.7} />
        </G>
        <G fill="#75d6ff">
          <AnimatedEllipse cx={5.3} cy={bubbleY(bubble1, 44)} rx={1.7} ry={1.8} opacity={bubbleOpacity(bubble1)} />
          <AnimatedEllipse cx={6.3} cy={bubbleY(bubble2, 23.4)} rx={4.3} ry={4.5} opacity={bubbleOpacity(bubble2)} />
          <AnimatedEllipse cx={12.8} cy={bubbleY(bubble3, 10.3)} rx={8} ry={8.3} opacity={bubbleOpacity(bubble3)} />
        </G>
        <Ellipse cx={18.7} cy={38.5} rx={7.1} ry={7.4} fill="#fcfcfa" />
        <AnimatedEllipse cx={18.7} cy={38.5} rx={4.9} ry={Animated.multiply(blink, 5.1)} fill="#29251c" />
      </Svg>
    </Animated.View>
  )
}
