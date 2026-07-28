import { View } from 'react-native'
import Svg, { Defs, LinearGradient, Path, Stop } from 'react-native-svg'

// Ported from frontend/src/pages/DeviceDashboardPage.tsx's Sparkline().
export function Sparkline({ data, id, color = '#667eea' }: { data: number[]; id: string; color?: string }) {
  if (data.length < 2) return null
  const W = 100
  const H = 36
  const max = Math.max(...data, 1)
  const pts: [number, number][] = data.map((v, i) => [(i / (data.length - 1)) * W, H - (v / max) * (H - 6) - 3])

  let linePath = `M ${pts[0][0]},${pts[0][1]}`
  for (let i = 1; i < pts.length; i++) {
    const dx = (pts[i][0] - pts[i - 1][0]) / 2.5
    linePath += ` C ${pts[i - 1][0] + dx},${pts[i - 1][1]} ${pts[i][0] - dx},${pts[i][1]} ${pts[i][0]},${pts[i][1]}`
  }
  const fillPath = `${linePath} L ${W},${H} L 0,${H} Z`
  const gradId = `sg-${id}`

  return (
    <View style={{ width: '100%', height: 40, marginTop: 8 }}>
      <Svg width="100%" height={40} viewBox={`0 0 ${W} ${H}`} preserveAspectRatio="none">
        <Defs>
          <LinearGradient id={gradId} x1="0" y1="0" x2="0" y2="1">
            <Stop offset="0%" stopColor={color} stopOpacity={0.35} />
            <Stop offset="100%" stopColor={color} stopOpacity={0} />
          </LinearGradient>
        </Defs>
        <Path d={fillPath} fill={`url(#${gradId})`} />
        <Path d={linePath} fill="none" stroke={color} strokeWidth={1.8} strokeLinecap="round" strokeLinejoin="round" />
      </Svg>
    </View>
  )
}
