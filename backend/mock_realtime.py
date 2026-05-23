"""Thin wrappers used by mock_device endpoints — delegate to realtime.py."""

try:
    from .activity import record_feed, touch_last_seen
except ImportError:
    from activity import record_feed, touch_last_seen


def mock_touch_last_seen(user_id: str, device_id: str | None = None) -> None:
    touch_last_seen(user_id, device_id)


def mock_record_feed(user_id: str, repeats: int, device_id: str | None = None) -> None:
    record_feed(user_id, repeats, device_id)
