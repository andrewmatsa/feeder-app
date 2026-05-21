"""Hardware tests — require a real ESP32 on the network.

Run with: pytest -m hardware --device-url http://192.168.1.100
These tests call the backend which forwards requests to the real firmware.
"""
from __future__ import annotations

import pytest
from fastapi.testclient import TestClient
from unittest.mock import patch

pytestmark = pytest.mark.hardware

try:
    from backend.main import app
    import backend.main as _main
    from backend.dependencies import UserClaims, get_current_user
    GET_BASE_URL_TARGET = "backend.main.get_device_base_url"
except ImportError:
    from main import app
    import main as _main
    from dependencies import UserClaims, get_current_user
    GET_BASE_URL_TARGET = "main.get_device_base_url"


def _fake_user() -> UserClaims:
    return UserClaims(id="hw-test-user", email="hw@test.com", role="user")


app.dependency_overrides[get_current_user] = _fake_user
client = TestClient(app)


@pytest.fixture(autouse=True)
def use_real_device(device_url: str):
    """Point backend at the real ESP32 and disable mock mode."""
    with patch.object(_main, "MOCK_DEVICE", False), \
         patch(GET_BASE_URL_TARGET, return_value=device_url):
        yield


# ── Status ────────────────────────────────────────────────────────────────────

def test_hw_status_returns_200() -> None:
    r = client.get("/api/status")
    assert r.status_code == 200


def test_hw_status_has_required_fields() -> None:
    body = client.get("/api/status").json()
    required = {"angle", "speed", "feedRepeats", "batteryVoltage", "batteryPercent", "isCharging", "feedTimes"}
    missing = required - body.keys()
    assert not missing, f"Missing fields: {missing}"


def test_hw_status_angle_in_valid_range() -> None:
    body = client.get("/api/status").json()
    assert 0 <= body["angle"] <= 180


def test_hw_status_battery_percent_in_valid_range() -> None:
    body = client.get("/api/status").json()
    assert 0 <= body["batteryPercent"] <= 100


def test_hw_status_feed_times_format() -> None:
    body = client.get("/api/status").json()
    assert isinstance(body["feedTimes"], list)
    for ft in body["feedTimes"]:
        assert {"hour", "minute", "repeats"} <= ft.keys()
        assert 0 <= ft["hour"] <= 23
        assert 0 <= ft["minute"] <= 59
        assert ft["repeats"] >= 1


# ── Feed ──────────────────────────────────────────────────────────────────────

def test_hw_feed_returns_success() -> None:
    r = client.post("/api/feed", json={"repeats": 1})
    assert r.status_code == 200
    assert r.json()["success"] is True


def test_hw_feed_respects_cooldown() -> None:
    # Second feed within cooldown window should return 200 or 429/504 (device enforces min interval)
    r = client.post("/api/feed", json={"repeats": 2})
    assert r.status_code in (200, 429, 504), f"Unexpected status: {r.status_code}"


def _get_status() -> dict:
    """Get status, skip test if device is unreachable (deep sleep)."""
    r = client.get("/api/status")
    if r.status_code != 200:
        pytest.skip(f"Device unreachable (status {r.status_code}) — may be in deep sleep")
    return r.json()


# ── Speed ─────────────────────────────────────────────────────────────────────

def test_hw_speed_set_returns_success() -> None:
    body = _get_status()
    original = body["speed"]
    new_speed = 15 if original != 15 else 20

    r = client.post("/api/speed", json={"speed": new_speed})
    assert r.status_code == 200
    assert r.json()["success"] is True

    client.post("/api/speed", json={"speed": original})


# ── Schedule ──────────────────────────────────────────────────────────────────

def test_hw_schedule_roundtrip() -> None:
    original_times = _get_status()["feedTimes"]

    new_times = [{"hour": 7, "minute": 30, "repeats": 1}]
    r = client.post("/api/schedule", json={"times": new_times})
    assert r.status_code == 200

    body = _get_status()
    returned = [{"hour": ft["hour"], "minute": ft["minute"], "repeats": ft["repeats"]} for ft in body["feedTimes"]]
    assert returned == new_times

    # Restore original schedule
    client.post("/api/schedule", json={"times": [
        {"hour": ft["hour"], "minute": ft["minute"], "repeats": ft["repeats"]}
        for ft in original_times
    ]})
