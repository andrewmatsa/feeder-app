// Talks directly to the feeder's own AP-mode web server (192.168.4.1), NOT
// the backend. This is a deliberate, scoped exception to the app's normal
// frontend -> backend -> firmware rule: while the phone is joined to the
// device's FishFeeder-XXXX hotspot it has no upstream internet, so the
// backend is categorically unreachable. src/web/wifi_page.cpp already talks
// to this same on-device API directly for the same reason.

const AP_BASE_URL = 'http://192.168.4.1'

export type ProvisionStatus = 'idle' | 'pending' | 'connecting' | 'connected' | 'failed'

export interface ProvisionStatusResponse {
  ok: boolean
  status: ProvisionStatus
  ssid?: string
  ip?: string
  message?: string
}

// AbortSignal.timeout() isn't reliably available in Hermes, so build the
// abort manually instead of relying on it.
function timeoutSignal(ms: number): AbortSignal {
  const controller = new AbortController()
  setTimeout(() => controller.abort(), ms)
  return controller.signal
}

// Short-timeout reachability probe used to detect that the phone has joined
// the device's hotspot, without needing OS-level SSID/location permissions.
export async function probeDeviceReachable(): Promise<boolean> {
  try {
    const res = await fetch(`${AP_BASE_URL}/api/provisionWiFiStatus`, {
      signal: timeoutSignal(1500),
    })
    return res.ok
  } catch {
    return false
  }
}

export async function submitWifiCredentials(ssid: string, password: string): Promise<void> {
  const body = new URLSearchParams({ ssid, password }).toString()
  const res = await fetch(`${AP_BASE_URL}/api/provisionWiFi`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
      'X-AquaFeed-Client': 'webui',
    },
    body,
    signal: timeoutSignal(5000),
  })
  const data = await res.json().catch(() => null)
  if (!res.ok || !data?.ok) {
    throw new Error(data?.message || 'connect_failed')
  }
}

export async function fetchProvisionStatus(): Promise<ProvisionStatusResponse> {
  const res = await fetch(`${AP_BASE_URL}/api/provisionWiFiStatus`, {
    signal: timeoutSignal(3000),
  })
  const data = await res.json()
  return data as ProvisionStatusResponse
}
