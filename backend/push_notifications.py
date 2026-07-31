"""Expo push notifications: token storage, and a background loop that checks
paired devices for low-battery / offline conditions and sends alerts.

New in this project — no existing push/notification code anywhere in
frontend/ or backend/ prior to this (mobile-only feature; the web SPA has no
equivalent and is unaffected)."""

from __future__ import annotations

import asyncio
from datetime import datetime, timedelta, timezone
from typing import Any

import httpx

try:
    from .config import MOCK_DEVICE, SUPABASE_SERVICE_KEY, SUPABASE_URL
    from .device_client import get_device_base_url, request_firmware
    from .mappers import map_firmware_status
except ImportError:
    from config import MOCK_DEVICE, SUPABASE_SERVICE_KEY, SUPABASE_URL
    from device_client import get_device_base_url, request_firmware
    from mappers import map_firmware_status

EXPO_PUSH_URL = "https://exp.host/--/api/v2/push/send"
CHECK_INTERVAL_SEC = 300  # 5 min — server-side polling only; does not touch the device's own sleep/wake cycle
RENOTIFY_COOLDOWN = timedelta(hours=6)
LOW_BATTERY_THRESHOLD = 20

# Deliberately out of scope for this pass: "missed scheduled feed" alerts.
# That needs comparing the schedule against actual feed_events over time,
# which is a heavier feature — battery-low and offline cover the two clearest,
# lowest-ambiguity signals for a v1. See plan.md Phase 5 notes.


def _service_client():
    from supabase import create_client
    return create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)


async def send_expo_push(tokens: list[str], title: str, body: str, data: dict[str, Any] | None = None) -> None:
    if not tokens:
        return
    messages = [{"to": t, "title": title, "body": body, "data": data or {}} for t in tokens]
    try:
        async with httpx.AsyncClient(timeout=10.0) as client:
            await client.post(EXPO_PUSH_URL, json=messages, headers={"Content-Type": "application/json"})
    except Exception:
        pass  # best-effort; a failed push must never break the alert loop


def register_push_token(user_id: str, device_id: str, token: str) -> None:
    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return
    _service_client().table("push_tokens").upsert(
        {"user_id": user_id, "device_id": device_id, "token": token, "platform": "expo"},
        on_conflict="device_id,token",
    ).execute()


def _devices_with_tokens() -> list[dict]:
    result = (
        _service_client()
        .table("devices")
        .select("id, settings, push_tokens(token)")
        .execute()
    )
    return [row for row in (result.data or []) if row.get("push_tokens")]


def _alert_state(settings: dict | None) -> dict:
    return dict((settings or {}).get("alerts", {}))


def _should_notify(state: dict, key: str) -> bool:
    ts = state.get(key)
    if not ts:
        return True
    try:
        last = datetime.fromisoformat(ts)
    except ValueError:
        return True
    return datetime.now(timezone.utc) - last >= RENOTIFY_COOLDOWN


def _set_alert(device_id: str, settings: dict | None, key: str, active: bool) -> None:
    alerts = _alert_state(settings)
    changed = False
    if active and key not in alerts:
        alerts[key] = datetime.now(timezone.utc).isoformat()
        changed = True
    elif not active and key in alerts:
        alerts.pop(key)
        changed = True
    if not changed:
        return
    new_settings = dict(settings or {})
    new_settings["alerts"] = alerts
    _service_client().table("devices").update({"settings": new_settings}).eq("id", device_id).execute()


async def _check_device(device: dict) -> None:
    device_id = device["id"]
    tokens = [t["token"] for t in device.get("push_tokens", [])]
    if not tokens:
        return
    settings = device.get("settings") or {}
    state = _alert_state(settings)

    try:
        response = await request_firmware("/api/status", base_url=get_device_base_url(device_id))
        status = map_firmware_status(response.json())
    except Exception:
        if _should_notify(state, "offline"):
            await send_expo_push(
                tokens, "AquaFeed", "Пристрій офлайн — не вдається підключитися",
                {"deviceId": device_id, "type": "offline"},
            )
        _set_alert(device_id, settings, "offline", True)
        return

    _set_alert(device_id, settings, "offline", False)

    is_low = status.batteryPercent < LOW_BATTERY_THRESHOLD and not status.isCharging
    if is_low and _should_notify(state, "lowBattery"):
        await send_expo_push(
            tokens, "AquaFeed", f"Низький заряд батареї: {status.batteryPercent}%",
            {"deviceId": device_id, "type": "lowBattery"},
        )
    _set_alert(device_id, settings, "lowBattery", is_low)


async def run_alert_check_loop() -> None:
    if MOCK_DEVICE or not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return  # nothing real to poll; avoid spamming mock/unconfigured deployments
    while True:
        try:
            for device in _devices_with_tokens():
                await _check_device(device)
        except Exception:
            pass  # never let one bad cycle kill the loop
        await asyncio.sleep(CHECK_INTERVAL_SEC)
