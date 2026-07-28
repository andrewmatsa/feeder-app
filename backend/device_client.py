"""HTTP client for the ESP32 firmware API."""

from __future__ import annotations

from typing import Any

import httpx
from fastapi import HTTPException

try:
    from .config import ESP32_BASE_URL, SUPABASE_SERVICE_KEY, SUPABASE_URL
except ImportError:
    from config import ESP32_BASE_URL, SUPABASE_SERVICE_KEY, SUPABASE_URL

http_client = httpx.AsyncClient(timeout=5.0, follow_redirects=False)
MUTATION_HEADERS = {"X-AquaFeed-Client": "backend"}


def get_device_base_url(device_id: str | None) -> str | None:
    """Return the ESP32 endpoint URL stored for this device, or None to use the default."""
    if not device_id or not SUPABASE_SERVICE_KEY or not SUPABASE_URL:
        return None
    try:
        from supabase import create_client
        client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
        result = (
            client.table("devices")
            .select("endpoint_url")
            .eq("id", device_id)
            .maybe_single()
            .execute()
        )
        if result.data:
            return result.data.get("endpoint_url") or None
    except Exception:
        pass
    return None


def register_device_ip(mac_address: str, ip: str) -> int:
    """Update endpoint_url + last_seen for every device matching this MAC.

    Called from an unauthenticated endpoint (firmware has no user token), so this
    uses the service-role client directly rather than DeviceService — same pattern
    as get_device_base_url. Returns the number of device rows updated (0 if the
    device hasn't been claimed in any account yet, or its mac_address is unknown).
    """
    if not SUPABASE_SERVICE_KEY or not SUPABASE_URL:
        return 0
    try:
        from supabase import create_client
        client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
        from datetime import datetime, timezone
        result = (
            client.table("devices")
            .update({
                "endpoint_url": f"http://{ip}",
                "last_seen": datetime.now(timezone.utc).isoformat(),
            })
            .eq("mac_address", mac_address.strip().lower())
            .execute()
        )
        return len(result.data or [])
    except Exception:
        return 0


async def request_firmware(
    path: str,
    *,
    method: str = "GET",
    params: dict[str, Any] | None = None,
    data: dict[str, Any] | None = None,
    base_url: str | None = None,
) -> httpx.Response:
    try:
        response = await http_client.request(
            method,
            f"{base_url or ESP32_BASE_URL}{path}",
            params=params,
            data=data,
            headers=MUTATION_HEADERS if method.upper() != "GET" else None,
        )
        response.raise_for_status()
        return response
    except httpx.TimeoutException as exc:
        raise HTTPException(status_code=504, detail="ESP32 request timed out") from exc
    except httpx.HTTPStatusError as exc:
        raise HTTPException(
            status_code=502,
            detail=f"ESP32 rejected the request: {exc.response.text or exc.response.reason_phrase}",
        ) from exc
    except httpx.RequestError as exc:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {exc}") from exc


async def close_http_client() -> None:
    await http_client.aclose()
