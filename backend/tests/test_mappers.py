from __future__ import annotations

try:
    from backend.mappers import encode_schedule, map_firmware_status, normalize_feed_times
    from backend.models import FeedTime
except ImportError:
    from mappers import encode_schedule, map_firmware_status, normalize_feed_times
    from models import FeedTime


def test_normalize_feed_times_supports_short_and_long_keys() -> None:
    raw = [
        {"h": 8, "m": 30, "r": 2},
        {"hour": 25, "minute": -5, "repeats": 0},
        "invalid-item",
    ]

    result = normalize_feed_times(raw)

    assert len(result) == 2
    assert result[0].hour == 8
    assert result[0].minute == 30
    assert result[0].repeats == 2
    assert result[1].hour == 23
    assert result[1].minute == 0
    assert result[1].repeats == 1


def test_map_firmware_status_applies_defaults_and_clamps() -> None:
    payload = {
        "currentAngle": 999,
        "speed": "12",
        "batteryPercent": 150,
        "nextFeedMinutes": -1,
        "feedTimes": [{"h": 10, "m": 0, "r": 1}],
    }

    mapped = map_firmware_status(payload)

    assert mapped.angle == 180
    assert mapped.speed == 12
    assert mapped.batteryPercent == 100
    assert mapped.nextFeedMinutes is None
    assert len(mapped.feedTimes) == 1
    assert mapped.feedTimes[0].hour == 10
    assert mapped.feedTimes[0].minute == 0
    assert mapped.feedTimes[0].repeats == 1


def test_encode_schedule_uses_firmware_compact_shape() -> None:
    schedule = [FeedTime(hour=10, minute=0, repeats=2)]

    encoded = encode_schedule(schedule)

    assert encoded == '[{"h":10,"m":0,"r":2}]'
