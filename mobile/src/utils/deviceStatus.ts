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

// Dashboard screens poll /api/status every 5s while open and write last_seen
// on each success, so 60s is already several missed polls, not a hair
// trigger — kept short so device lists reflect a power-off promptly instead
// of lagging minutes behind what the dashboard itself would already show.
export function isDeviceOnline(lastSeen: string | null | undefined): boolean {
  if (!lastSeen) return false
  const diffMs = Date.now() - new Date(lastSeen).getTime()
  return diffMs < 60_000
}
