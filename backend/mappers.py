"""Mapping helpers between firmware payloads and backend API models."""

from __future__ import annotations

from datetime import datetime
import json
from typing import Any

try:
    from .models import FeedTime, StatusResponse
except ImportError:
    from models import FeedTime, StatusResponse


def to_int(value: Any, default: int = 0) -> int:
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def to_float(value: Any, default: float = 0.0) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def to_optional_non_negative_int(value: Any) -> int | None:
    parsed = to_int(value, -1)
    return parsed if parsed >= 0 else None


def normalize_feed_times(raw_feed_times: Any) -> list[FeedTime]:
    normalized: list[FeedTime] = []
    if not isinstance(raw_feed_times, list):
        return normalized

    for item in raw_feed_times:
        if not isinstance(item, dict):
            continue
        normalized.append(
            FeedTime(
                hour=max(0, min(23, to_int(item.get("h", item.get("hour")), 0))),
                minute=max(0, min(59, to_int(item.get("m", item.get("minute")), 0))),
                repeats=max(1, to_int(item.get("r", item.get("repeats")), 1)),
            )
        )
    return normalized


def map_firmware_status(payload: dict[str, Any]) -> StatusResponse:
    return StatusResponse(
        angle=max(0, min(180, to_int(payload.get("currentAngle", payload.get("angle")), 0))),
        speed=max(1.0, min(20.0, to_float(payload.get("speed"), 20.0))),
        feedRepeats=max(1, to_int(payload.get("feedRepeats"), 1)),
        minFeedIntervalMin=max(1, to_int(payload.get("minFeedIntervalMin"), 5)),
        powerSaveMode=bool(payload.get("powerSaveMode", False)),
        displayEnabled=bool(payload.get("displayEnabled", True)),
        displayOffAfterSec=max(0, to_int(payload.get("displayOffAfterSec"), 20)),
        deepSleepIdleSec=max(0, to_int(payload.get("deepSleepIdleSec"), 60)),
        batteryVoltage=to_float(payload.get("batteryVoltage"), 0.0),
        batteryPercent=max(0, min(100, to_int(payload.get("batteryPercent"), 0))),
        isCharging=bool(payload.get("isCharging", False)),
        feedTimes=normalize_feed_times(payload.get("feedTimes")),
        nextFeedMinutes=to_optional_non_negative_int(payload.get("nextFeedMinutes")),
        nextFeedHour=to_optional_non_negative_int(payload.get("nextFeedHour")),
        nextFeedMinute=to_optional_non_negative_int(payload.get("nextFeedMinute")),
        currentTime=payload.get("currentTime"),
        manualFeedCooldownSeconds=max(0, to_int(payload.get("manualFeedCooldownSeconds"), 0)),
        wifiSSID=str(payload.get("wifiSSID") or ""),
        wifiIP=str(payload.get("wifiIP") or ""),
        isAPMode=bool(payload.get("isAPMode", False)),
        sleepReason=str(payload.get("sleepReason") or "unknown"),
        sleepCountdownSeconds=to_int(payload.get("sleepCountdownSeconds"), -1),
        displayAwake=bool(payload.get("displayAwake", True)),
        timestamp=datetime.now().isoformat(),
        lightLux=to_optional_non_negative_int(payload.get("lightLux")),
    )


def encode_schedule(times: list[FeedTime]) -> str:
    firmware_times = [{"h": time.hour, "m": time.minute, "r": time.repeats} for time in times]
    return json.dumps(firmware_times, separators=(",", ":"))
