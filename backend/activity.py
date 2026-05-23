"""Records real-time device activity to the database.

Called on every status poll and manual feed regardless of MOCK_DEVICE mode,
so the admin panel reflects accurate last_seen and feed history for real devices.
"""

from datetime import datetime, timezone

from supabase import create_client

try:
    from .config import SUPABASE_SERVICE_KEY, SUPABASE_URL
except ImportError:
    from config import SUPABASE_SERVICE_KEY, SUPABASE_URL


def _client():
    if not SUPABASE_SERVICE_KEY or not SUPABASE_URL:
        return None
    return create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)


def touch_last_seen(user_id: str, device_id: str | None = None) -> None:
    """Update last_seen = NOW() for a specific device, or all user devices if device_id is None."""
    client = _client()
    if not client:
        return
    try:
        q = client.table("devices").update(
            {"last_seen": datetime.now(timezone.utc).isoformat()}
        ).eq("user_id", user_id)
        if device_id:
            q = q.eq("id", device_id)
        q.execute()
    except Exception:
        pass


def record_feed(user_id: str, repeats: int, device_id: str | None = None) -> None:
    """Insert a feed_event row and persist the repeat count in device settings."""
    client = _client()
    if not client:
        return
    try:
        if not device_id:
            result = (
                client.table("devices")
                .select("id")
                .eq("user_id", user_id)
                .limit(1)
                .execute()
            )
            if not result.data:
                return
            device_id = result.data[0]["id"]
        client.table("feed_events").insert({
            "device_id": device_id,
            "user_id": user_id,
            "source": "manual",
            "repeats": repeats,
        }).execute()
        try:
            row = client.table("devices").select("settings").eq("id", device_id).maybe_single().execute()
            settings = (row.data or {}).get("settings") or {}
            settings["feedRepeats"] = repeats
            client.table("devices").update({"settings": settings}).eq("id", device_id).execute()
        except Exception:
            pass
    except Exception:
        pass
