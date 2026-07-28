import { StyleSheet, Text, View } from 'react-native'
import Svg, { Path, Text as SvgText } from 'react-native-svg'

const GAUGE_R = 110
const GAUGE_CX = 130
const GAUGE_CY = 140
const GAUGE_START = 180
const GAUGE_ARC = 180
const CIRCUMFERENCE = 2 * Math.PI * GAUGE_R
const TRACK_LEN = (GAUGE_ARC / 360) * CIRCUMFERENCE

function describeArc(cx: number, cy: number, r: number, startDeg: number, endDeg: number): string {
  const toRad = (d: number) => (d * Math.PI) / 180
  const x1 = cx + r * Math.cos(toRad(startDeg))
  const y1 = cy + r * Math.sin(toRad(startDeg))
  const x2 = cx + r * Math.cos(toRad(endDeg))
  const y2 = cy + r * Math.sin(toRad(endDeg))
  const large = endDeg - startDeg > 180 ? 1 : 0
  return `M ${x1} ${y1} A ${r} ${r} 0 ${large} 1 ${x2} ${y2}`
}

const TRACK_PATH = describeArc(GAUGE_CX, GAUGE_CY, GAUGE_R, GAUGE_START, GAUGE_START + GAUGE_ARC)

function GaugeSvg({ fraction, color, label }: { fraction: number; color: string; label: string }) {
  const clamped = Math.max(0, Math.min(1, fraction))
  const offset = TRACK_LEN * (1 - clamped)
  return (
    // width:'100%' + aspectRatio (not a fixed pixel width) lets this shrink to fit
    // however many gauges share the row — matches web's .aq-gauge-svg { width:100%; height:auto }.
    // A fixed width here is what caused 2-3 gauges to overflow and overlap.
    <Svg width="100%" viewBox="0 0 260 160" style={{ aspectRatio: 260 / 160 }}>
      <Path d={TRACK_PATH} fill="none" stroke="#E6E9EF" strokeWidth={14} strokeLinecap="round" />
      <Path
        d={TRACK_PATH}
        fill="none"
        stroke={color}
        strokeWidth={14}
        strokeLinecap="round"
        strokeDasharray={`${TRACK_LEN}`}
        strokeDashoffset={offset}
      />
      <SvgText x={130} y={138} textAnchor="middle" fontSize={28} fontWeight="700" fill={color}>
        {label}
      </SvgText>
    </Svg>
  )
}

export function BatteryGauge({
  percent,
  voltage,
  title,
  voltageLabel,
}: {
  percent: number
  voltage: number
  title: string
  voltageLabel: string
}) {
  const clamp = Math.max(0, Math.min(100, percent))
  const color = clamp >= 50 ? '#4CAF50' : clamp >= 20 ? '#FF9800' : '#f44336'
  return (
    <View style={styles.wrap}>
      <GaugeSvg fraction={clamp / 100} color={color} label={`${clamp}%`} />
      <Text style={styles.title}>{title}</Text>
      <Text style={styles.sub}>
        {voltageLabel} {voltage.toFixed(2)} V
      </Text>
    </View>
  )
}

export function NextFeedGauge({
  nextFeedMinutes,
  title,
  formatLabel,
}: {
  nextFeedMinutes: number | null | undefined
  title: string
  formatLabel: (h: number, m: number) => string
}) {
  const MAX_MINUTES = 24 * 60
  const minutes = nextFeedMinutes ?? null
  const fraction = minutes != null ? 1 - minutes / MAX_MINUTES : 0
  const h = minutes != null ? Math.floor(minutes / 60) : 0
  const m = minutes != null ? minutes % 60 : 0
  const label = minutes != null ? formatLabel(h, m) : '--'
  return (
    <View style={styles.wrap}>
      <GaugeSvg fraction={fraction} color="#1976D2" label={label} />
      <Text style={styles.title}>{title}</Text>
    </View>
  )
}

export function LightGauge({
  durationSeconds,
  isOn,
  title,
  onLabel,
  offLabel,
}: {
  durationSeconds: number
  isOn: boolean
  title: string
  onLabel: string
  offLabel: string
}) {
  const MAX_SECONDS = 60 * 60
  const fraction = isOn ? Math.min(1, durationSeconds / MAX_SECONDS) : 0
  const color = isOn ? '#D97706' : '#9CA3AF'
  const minutes = Math.floor(durationSeconds / 60)
  const label = isOn ? `${minutes} хв` : '--'
  return (
    <View style={styles.wrap}>
      <GaugeSvg fraction={fraction} color={color} label={label} />
      <Text style={styles.title}>{title}</Text>
      <Text style={[styles.sub, isOn && styles.subOn]}>{isOn ? onLabel : offLabel}</Text>
    </View>
  )
}

const styles = StyleSheet.create({
  wrap: { alignItems: 'center', flex: 1, minWidth: 0, maxWidth: 160 },
  title: { fontSize: 11, fontWeight: '600', color: '#333', marginTop: 4, textAlign: 'center' },
  sub: { fontSize: 10, color: '#888', marginTop: 2, textAlign: 'center' },
  subOn: { color: '#D97706', fontWeight: '700' },
})
