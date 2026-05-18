export function formatLastSeen(lastSeen: string | null | undefined): string {
  if (!lastSeen) return 'Ще не синхронізовано'

  const seen = new Date(lastSeen)
  const diffMs = Date.now() - seen.getTime()
  const diffMin = Math.floor(diffMs / 60_000)

  if (diffMin < 1) return 'Щойно'
  if (diffMin < 60) return `${diffMin} хв тому`
  const diffHours = Math.floor(diffMin / 60)
  if (diffHours < 24) return `${diffHours} год тому`
  return seen.toLocaleDateString('uk-UA')
}

export function isDeviceOnline(lastSeen: string | null | undefined): boolean {
  if (!lastSeen) return false
  const diffMs = Date.now() - new Date(lastSeen).getTime()
  return diffMs < 5 * 60_000
}
