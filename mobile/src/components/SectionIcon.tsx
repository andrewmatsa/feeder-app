import { StyleSheet, View } from 'react-native'
import Svg, { Circle, Line, Path, Polyline, Rect } from 'react-native-svg'

// Ported from frontend/src/pages/DeviceDashboardPage.tsx's per-card <div className="aq-section-icon">
// SVGs, and the .aq-section-icon box styling in App.css (36x36, #EEF1F6 bg, 20x20 stroke #4A5568 icon).
export type SectionIconName =
  | 'clock'
  | 'calendar'
  | 'servo'
  | 'wifi'
  | 'battery'
  | 'gear'
  | 'monitor'
  | 'grid'
  | 'document'
  | 'globe'
  | 'power'
  | 'calibration'
  | 'cloudUpload'
  | 'sun'
  | 'list'

const ICON_PROPS = { viewBox: '0 0 24 24', fill: 'none', stroke: '#4A5568', strokeWidth: 1.8, strokeLinecap: 'round', strokeLinejoin: 'round' } as const

function IconPaths({ name }: { name: SectionIconName }) {
  switch (name) {
    case 'clock':
      return (
        <>
          <Path d="M12 2a10 10 0 1 0 0 20A10 10 0 0 0 12 2z" />
          <Path d="M12 8v4l3 3" />
        </>
      )
    case 'calendar':
      return (
        <>
          <Rect x={3} y={4} width={18} height={18} rx={2} ry={2} />
          <Line x1={16} y1={2} x2={16} y2={6} />
          <Line x1={8} y1={2} x2={8} y2={6} />
          <Line x1={3} y1={10} x2={21} y2={10} />
        </>
      )
    case 'servo':
      return (
        <>
          <Line x1={8} y1={5} x2={8} y2={19} />
          <Line x1={16} y1={5} x2={16} y2={19} />
          <Circle cx={8} cy={10} r={2.5} />
          <Circle cx={16} cy={14} r={2.5} />
        </>
      )
    case 'wifi':
      return (
        <>
          <Path d="M5 12.55a11 11 0 0 1 14.08 0" />
          <Path d="M1.42 9a16 16 0 0 1 21.16 0" />
          <Path d="M8.53 16.11a6 6 0 0 1 6.95 0" />
          <Line x1={12} y1={20} x2={12.01} y2={20} />
        </>
      )
    case 'battery':
      return (
        <>
          <Rect x={2} y={7} width={16} height={10} rx={2} ry={2} />
          <Line x1={22} y1={11} x2={22} y2={13} />
        </>
      )
    case 'gear':
      return (
        <>
          <Circle cx={12} cy={12} r={3} />
          <Path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
        </>
      )
    case 'monitor':
      return (
        <>
          <Rect x={2} y={3} width={20} height={14} rx={2} ry={2} />
          <Line x1={8} y1={21} x2={16} y2={21} />
          <Line x1={12} y1={17} x2={12} y2={21} />
        </>
      )
    case 'grid':
      return (
        <>
          <Rect x={3} y={3} width={18} height={18} rx={2} />
          <Line x1={9} y1={9} x2={15} y2={9} />
          <Line x1={9} y1={12} x2={15} y2={12} />
          <Line x1={9} y1={15} x2={13} y2={15} />
        </>
      )
    case 'document':
      return (
        <>
          <Path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z" />
          <Polyline points="14 2 14 8 20 8" />
          <Line x1={16} y1={13} x2={8} y2={13} />
          <Line x1={16} y1={17} x2={8} y2={17} />
          <Polyline points="10 9 9 9 8 9" />
        </>
      )
    case 'globe':
      return (
        <>
          <Circle cx={12} cy={12} r={10} />
          <Line x1={2} y1={12} x2={22} y2={12} />
          <Path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z" />
        </>
      )
    case 'power':
      return (
        <>
          <Path d="M18.36 6.64a9 9 0 1 1-12.73 0" />
          <Line x1={12} y1={2} x2={12} y2={12} />
        </>
      )
    case 'calibration':
      return (
        <>
          <Line x1={8} y1={6} x2={21} y2={6} />
          <Line x1={8} y1={12} x2={21} y2={12} />
          <Line x1={8} y1={18} x2={21} y2={18} />
          <Line x1={3} y1={6} x2={3.01} y2={6} />
          <Line x1={3} y1={12} x2={3.01} y2={12} />
          <Line x1={3} y1={18} x2={3.01} y2={18} />
        </>
      )
    case 'cloudUpload':
      return (
        <>
          <Polyline points="16 16 12 12 8 16" />
          <Line x1={12} y1={12} x2={12} y2={21} />
          <Path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3" />
        </>
      )
    case 'sun':
      return (
        <>
          <Circle cx={12} cy={12} r={5} />
          <Line x1={12} y1={1} x2={12} y2={3} />
          <Line x1={12} y1={21} x2={12} y2={23} />
          <Line x1={4.22} y1={4.22} x2={5.64} y2={5.64} />
          <Line x1={18.36} y1={18.36} x2={19.78} y2={19.78} />
          <Line x1={1} y1={12} x2={3} y2={12} />
          <Line x1={21} y1={12} x2={23} y2={12} />
          <Line x1={4.22} y1={19.78} x2={5.64} y2={18.36} />
          <Line x1={18.36} y1={5.64} x2={19.78} y2={4.22} />
        </>
      )
    case 'list':
      return <Path d="M3 6h18M3 12h18M3 18h18" />
  }
}

export function SectionIcon({ name }: { name: SectionIconName }) {
  return (
    <View style={styles.box}>
      <Svg width={20} height={20} {...ICON_PROPS}>
        <IconPaths name={name} />
      </Svg>
    </View>
  )
}

const styles = StyleSheet.create({
  box: {
    width: 36,
    height: 36,
    borderRadius: 12,
    backgroundColor: '#EEF1F6',
    alignItems: 'center',
    justifyContent: 'center',
  },
})
