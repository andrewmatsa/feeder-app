import Svg, { Line, Path, Polyline } from 'react-native-svg'

// Matches the stroke-icon language already used in TabIcons.tsx/SectionIcon.tsx.
const ICON_PROPS = { viewBox: '0 0 24 24', fill: 'none', strokeWidth: 2, strokeLinecap: 'round', strokeLinejoin: 'round' } as const

export function EditIcon({ color, size = 17 }: { color: string; size?: number }) {
  return (
    <Svg width={size} height={size} {...ICON_PROPS} stroke={color}>
      <Path d="M17 3a2.85 2.83 0 1 1 4 4L7.5 20.5 2 22l1.5-5.5Z" />
      <Path d="M15 5l4 4" />
    </Svg>
  )
}

export function TrashIcon({ color, size = 17 }: { color: string; size?: number }) {
  return (
    <Svg width={size} height={size} {...ICON_PROPS} stroke={color}>
      <Polyline points="3 6 5 6 21 6" />
      <Path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6" />
      <Path d="M10 11v6" />
      <Path d="M14 11v6" />
      <Path d="M9 6V4a2 2 0 0 1 2-2h2a2 2 0 0 1 2 2v2" />
    </Svg>
  )
}

export function ChevronRightIcon({ color, size = 16 }: { color: string; size?: number }) {
  return (
    <Svg width={size} height={size} {...ICON_PROPS} stroke={color}>
      <Line x1={5} y1={12} x2={19} y2={12} />
      <Path d="M13 6l6 6-6 6" />
    </Svg>
  )
}
