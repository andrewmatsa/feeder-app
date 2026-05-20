"""Mock device responses for development without a physical ESP32.

Settings are persisted per-device in devices.settings (JSONB) so each device
maintains its own schedule, speed, angle, etc. Falls back to in-memory state
when Supabase credentials are not available.
"""

from datetime import datetime

from supabase import create_client

try:
    from .config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from .models import CommandResponse, FeedTime, StatusResponse
except ImportError:
    from config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from models import CommandResponse, FeedTime, StatusResponse

DEFAULT_SETTINGS: dict = {
    "speed": 20.0,
    "angle": 90,
    "feedRepeats": 1,
    "powerSaveMode": False,
    "deepSleepIdleSec": 120,
    "displayEnabled": True,
    "displayOffAfterSec": 30,
    "minFeedIntervalMin": 5,
    "timezoneOffset": 2,
    "batteryVoltage": 3.85,
    "batteryPercent": 72,
    "isCharging": True,
    "lightLux": 620,
    "wifiSSID": "HomeNetwork",
    "wifiIP": "192.168.1.42",
    "feedTimes": [
        {"hour": 8, "minute": 0, "repeats": 1},
        {"hour": 13, "minute": 30, "repeats": 2},
        {"hour": 19, "minute": 0, "repeats": 1},
    ],
}

# In-memory fallback when no Supabase client is available
_memory: dict[str, dict] = {}


def _client():
    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return None
    return create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)


def _get_settings(device_id: str | None) -> dict:
    """Load settings from DB merged over defaults. Falls back to in-memory."""
    base = {**DEFAULT_SETTINGS}
    if not device_id:
        return base

    client = _client()
    if not client:
        return {**base, **_memory.get(device_id, {})}

    try:
        result = (
            client.table("devices")
            .select("settings")
            .eq("id", device_id)
            .maybe_single()
            .execute()
        )
        if result.data and result.data.get("settings"):
            return {**base, **result.data["settings"]}
    except Exception:
        pass

    return base


def _save_settings(device_id: str | None, settings: dict) -> None:
    """Write settings dict back to DB."""
    if not device_id:
        return

    client = _client()
    if not client:
        _memory[device_id] = settings
        return

    try:
        client.table("devices").update({"settings": settings}).eq("id", device_id).execute()
    except Exception:
        _memory[device_id] = settings


def _patch(device_id: str | None, key: str, value) -> None:
    s = _get_settings(device_id)
    s[key] = value
    _save_settings(device_id, s)


# ─── Public mock functions ────────────────────────────────────────────────────

def mock_status(device_id: str | None = None) -> StatusResponse:
    s = _get_settings(device_id)
    now = datetime.now()

    feed_times = [FeedTime(**ft) for ft in s.get("feedTimes", DEFAULT_SETTINGS["feedTimes"])]

    now_min = now.hour * 60 + now.minute
    if feed_times:
        deltas = [(ft.hour * 60 + ft.minute - now_min) % (24 * 60) for ft in feed_times]
        idx = min(range(len(deltas)), key=lambda i: deltas[i])
        nf = feed_times[idx]
        next_feed_minutes = deltas[idx]
        next_feed_hour = nf.hour
        next_feed_minute = nf.minute
    else:
        next_feed_minutes = next_feed_hour = next_feed_minute = None

    return StatusResponse(
        angle=s["angle"],
        speed=s["speed"],
        feedRepeats=s["feedRepeats"],
        minFeedIntervalMin=s["minFeedIntervalMin"],
        powerSaveMode=s["powerSaveMode"],
        displayEnabled=s["displayEnabled"],
        displayOffAfterSec=s["displayOffAfterSec"],
        deepSleepIdleSec=s["deepSleepIdleSec"],
        batteryVoltage=s["batteryVoltage"],
        batteryPercent=s["batteryPercent"],
        isCharging=s["isCharging"],
        feedTimes=feed_times,
        nextFeedHour=next_feed_hour,
        nextFeedMinute=next_feed_minute,
        nextFeedMinutes=next_feed_minutes,
        currentTime=now.strftime("%H:%M:%S"),
        manualFeedCooldownSeconds=0,
        wifiSSID=s["wifiSSID"],
        wifiIP=s["wifiIP"],
        isAPMode=False,
        sleepReason="none",
        sleepCountdownSeconds=-1,
        displayAwake=True,
        timestamp=now.isoformat(),
        cpuFrequency=160,
        memoryFreeHeap=245320,
        memoryUsedHeap=154680,
        memoryTotalHeap=400000,
        memoryMaxAllocHeap=180500,
        memoryMinFreeHeap=200250,
        cacheSize=500,
        cacheAge=245,
        cacheValid=True,
        uptimeSeconds=8100,
        lightLux=s.get("lightLux"),
    )


def mock_feed(repeats: int) -> CommandResponse:
    return CommandResponse(success=True, message=f"[MOCK] Feeding started with {repeats} repeats")


def mock_set_speed(speed: int, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "speed", speed)
    return CommandResponse(success=True, message=f"[MOCK] Speed set to {speed}")


def mock_set_angle(angle: int, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "angle", angle)
    return CommandResponse(success=True, message=f"[MOCK] Angle set to {angle}")


def mock_set_schedule(times: list[FeedTime], device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "feedTimes", [ft.model_dump() for ft in times])
    return CommandResponse(success=True, message=f"[MOCK] Schedule updated with {len(times)} slots")


def mock_set_power_mode(enabled: bool, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "powerSaveMode", enabled)
    return CommandResponse(success=True, message=f"[MOCK] Power save {'enabled' if enabled else 'disabled'}")


def mock_set_display_settings(
    power_save: bool,
    deep_sleep_sec: int,
    display_enabled: bool,
    display_off_sec: int,
    device_id: str | None = None,
) -> CommandResponse:
    s = _get_settings(device_id)
    s["powerSaveMode"] = power_save
    s["deepSleepIdleSec"] = deep_sleep_sec
    s["displayEnabled"] = display_enabled
    s["displayOffAfterSec"] = display_off_sec
    _save_settings(device_id, s)
    return CommandResponse(success=True, message="[MOCK] Display settings updated")


def mock_set_min_interval(min_interval_min: int, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "minFeedIntervalMin", min_interval_min)
    return CommandResponse(success=True, message=f"[MOCK] Min interval set to {min_interval_min} min")


def mock_calibrate(actual_voltage: float, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "batteryVoltage", actual_voltage)
    return CommandResponse(success=True, message=f"[MOCK] Battery calibrated to {actual_voltage} V")


def mock_set_timezone(offset_hours: int, device_id: str | None = None) -> CommandResponse:
    _patch(device_id, "timezoneOffset", offset_hours)
    return CommandResponse(success=True, message=f"[MOCK] Timezone set to UTC{offset_hours:+d}")
