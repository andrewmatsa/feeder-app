"""Integration tests for FastAPI → mock_device flow.

Tests the full request/response cycle with MOCK_DEVICE=true and auth bypassed.
Run with: pytest -m mock
"""
from __future__ import annotations

from unittest.mock import patch

import pytest
from fastapi.testclient import TestClient

pytestmark = pytest.mark.mock

try:
    from backend.main import app, get_device_service
    import backend.main as _main
    import backend.mock_device as _mock_device
    from backend.dependencies import UserClaims, get_current_user
    from backend.device_service import InMemoryDeviceService
except ImportError:
    from main import app, get_device_service
    import main as _main
    import mock_device as _mock_device
    from dependencies import UserClaims, get_current_user
    from device_service import InMemoryDeviceService


# ── Auth bypass ──────────────────────────────────────────────────────────────

def _fake_user() -> UserClaims:
    return UserClaims(id="test-user-id", email="test@example.com", role="user")


app.dependency_overrides[get_current_user] = _fake_user
# get_status also depends on get_device_service (last_seen/mac self-heal),
# which has its own independent bearer-token dependency not covered by the
# get_current_user override above.
app.dependency_overrides[get_device_service] = lambda: InMemoryDeviceService()
client = TestClient(app)


# ── Fixtures ─────────────────────────────────────────────────────────────────

@pytest.fixture(autouse=True)
def mock_device_on():
    """Force MOCK_DEVICE=True and disable Supabase so in-memory state is used."""
    _mock_device._memory.clear()
    with patch.object(_main, "MOCK_DEVICE", True), \
         patch.object(_mock_device, "_client", return_value=None):
        yield
    _mock_device._memory.clear()


# ── Status endpoint ───────────────────────────────────────────────────────────

def test_status_returns_200() -> None:
    r = client.get("/api/status")
    assert r.status_code == 200


def test_status_has_required_fields() -> None:
    body = client.get("/api/status").json()
    required = {
        "angle", "speed", "feedRepeats", "batteryVoltage", "batteryPercent",
        "isCharging", "feedTimes", "powerSaveMode", "displayEnabled",
    }
    missing = required - body.keys()
    assert not missing, f"Missing fields: {missing}"


def test_status_no_legacy_fields() -> None:
    """Verify the removed legacy feedHour1/feedHour2 fields are not exposed."""
    body = client.get("/api/status").json()
    assert "feedHour1" not in body
    assert "feedHour2" not in body
    assert "feedMinute1" not in body
    assert "feedMinute2" not in body


def test_status_feed_times_format() -> None:
    """feedTimes must be a list of objects with hour/minute/repeats keys."""
    body = client.get("/api/status").json()
    assert isinstance(body["feedTimes"], list)
    assert len(body["feedTimes"]) > 0
    first = body["feedTimes"][0]
    assert {"hour", "minute", "repeats"} <= first.keys()


# ── Feed endpoint ─────────────────────────────────────────────────────────────

def test_feed_returns_success() -> None:
    r = client.post("/api/feed", json={"repeats": 1})
    assert r.status_code == 200
    assert r.json()["success"] is True


def test_feed_message_contains_repeats() -> None:
    r = client.post("/api/feed", json={"repeats": 3})
    assert "3" in r.json()["message"]


def test_feed_rejects_zero_repeats() -> None:
    r = client.post("/api/feed", json={"repeats": 0})
    assert r.status_code == 422


def test_feed_rejects_missing_body() -> None:
    r = client.post("/api/feed")
    assert r.status_code == 422


# ── Speed persistence ─────────────────────────────────────────────────────────

def test_speed_set_and_reflected_in_status() -> None:
    # Use a fixed device_id so mock in-memory state persists across requests
    did = "test-device-speed"
    r = client.post("/api/speed", json={"speed": 15}, params={"device_id": did})
    assert r.status_code == 200
    assert r.json()["success"] is True

    status = client.get("/api/status", params={"device_id": did}).json()
    assert status["speed"] == 15


# ── Schedule round-trip ───────────────────────────────────────────────────────

def test_schedule_roundtrip() -> None:
    did = "test-device-schedule"
    new_times = [
        {"hour": 7, "minute": 30, "repeats": 2},
        {"hour": 20, "minute": 0, "repeats": 1},
    ]
    r = client.post("/api/schedule", json={"times": new_times}, params={"device_id": did})
    assert r.status_code == 200

    status = client.get("/api/status", params={"device_id": did}).json()
    returned = [{k: ft[k] for k in ("hour", "minute", "repeats")} for ft in status["feedTimes"]]
    assert returned == new_times


def test_schedule_single_slot() -> None:
    did = "test-device-schedule-single"
    r = client.post(
        "/api/schedule",
        json={"times": [{"hour": 9, "minute": 15, "repeats": 3}]},
        params={"device_id": did},
    )
    assert r.status_code == 200

    status = client.get("/api/status", params={"device_id": did}).json()
    assert len(status["feedTimes"]) == 1
    assert status["feedTimes"][0]["hour"] == 9


# ── Timezone ──────────────────────────────────────────────────────────────────

def test_timezone_set() -> None:
    r = client.post("/api/timezone", json={"offsetHours": 3})
    assert r.status_code == 200
    assert r.json()["success"] is True


# ── Min interval ──────────────────────────────────────────────────────────────

def test_min_interval_set() -> None:
    r = client.post("/api/min-interval", json={"minFeedIntervalMin": 10})
    assert r.status_code == 200
    assert r.json()["success"] is True


# ── Power mode ────────────────────────────────────────────────────────────────

def test_power_mode_toggle() -> None:
    r = client.post("/api/power-mode", json={"enabled": True})
    assert r.status_code == 200
    assert r.json()["success"] is True
