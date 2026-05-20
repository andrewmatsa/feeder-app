"""Hardware integration tests — require a real ESP32 connected to WiFi.

These tests send real HTTP requests to the physical device. The servo WILL move.

Usage:
    pytest tests/test_hardware.py --device-url http://192.168.1.100 -v

All tests are skipped automatically when --device-url is not provided.
"""
from __future__ import annotations

import json
import time

import pytest
import requests

pytestmark = pytest.mark.hardware

AUTH = {"X-AquaFeed-Client": "backend"}
TIMEOUT = 15  # seconds — servo movement can take up to 10s at speed=1


# ── Connectivity ──────────────────────────────────────────────────────────────

def test_root_returns_json(device_url):
    r = requests.get(f"{device_url}/", timeout=5)
    assert r.status_code == 200
    assert "application/json" in r.headers.get("Content-Type", ""), (
        f"Expected JSON content-type, got: {r.headers.get('Content-Type')}"
    )
    body = r.json()
    assert "device" in body, f"Missing 'device' key, got: {list(body.keys())}"
    assert "<html" not in r.text.lower(), "Root returned HTML instead of JSON"


def test_status_required_fields(device_url):
    r = requests.get(f"{device_url}/api/status", timeout=5)
    assert r.status_code == 200
    body = r.json()
    required = {
        "status", "batteryVoltage", "batteryPercent", "feedTimes",
        "speed", "currentAngle", "feedRepeats", "powerSaveMode", "displayEnabled",
    }
    missing = required - body.keys()
    assert not missing, f"Missing fields in /api/status: {missing}"


def test_status_no_legacy_fields(device_url):
    body = requests.get(f"{device_url}/api/status", timeout=5).json()
    for legacy in ("feedHour1", "feedHour2", "feedMinute1", "feedMinute2"):
        assert legacy not in body, f"Legacy field '{legacy}' still present in status"


def test_feed_times_format(device_url):
    body = requests.get(f"{device_url}/api/status", timeout=5).json()
    times = body.get("feedTimes", [])
    assert isinstance(times, list), "feedTimes must be a list"
    assert len(times) > 0, "feedTimes must have at least one entry"
    for slot in times:
        assert "hour" in slot or "h" in slot, f"Slot missing hour key: {slot}"
        assert "minute" in slot or "m" in slot, f"Slot missing minute key: {slot}"


# ── Security ──────────────────────────────────────────────────────────────────

def test_feed_requires_auth_header(device_url):
    """POST without X-AquaFeed-Client header must be rejected."""
    r = requests.post(f"{device_url}/api/feedNow", timeout=5)
    assert r.status_code == 403, (
        f"Expected 403 Forbidden without auth header, got {r.status_code}. "
        "Security check failed — unauthenticated clients can trigger feeding!"
    )


# ── Servo — CRITICAL ──────────────────────────────────────────────────────────

def test_servo_rotates(device_url):
    """CRITICAL: servo must physically rotate when feedNow is called.

    If this test fails the servo is not moving on the real device.
    Check: wiring, power supply, servo_controller.cpp, ESP32 GPIO config.
    """
    r = requests.post(
        f"{device_url}/api/feedNow",
        headers=AUTH,
        data={"repeats": "1"},
        timeout=TIMEOUT,
    )
    assert r.status_code == 200, f"feedNow returned {r.status_code}: {r.text}"
    assert "feeding" in r.text.lower(), (
        f"Unexpected feedNow response: {r.text!r} — servo may not have moved"
    )


def test_servo_speed_timing(device_url):
    """CRITICAL: speed=1 must take significantly longer than speed=20.

    Verifies that speedToStepDelayMs() is wired up correctly and the servo
    actually slows down. A ratio < 5× means the speed slider has no effect.
    """
    # Slow pass
    requests.post(f"{device_url}/api/setSpeed", headers=AUTH, data={"speed": "1"}, timeout=5)
    time.sleep(0.5)
    t0 = time.time()
    requests.post(f"{device_url}/api/feedNow", headers=AUTH, data={"repeats": "1"}, timeout=30)
    slow_s = time.time() - t0

    # Fast pass
    requests.post(f"{device_url}/api/setSpeed", headers=AUTH, data={"speed": "20"}, timeout=5)
    time.sleep(0.5)
    t0 = time.time()
    requests.post(f"{device_url}/api/feedNow", headers=AUTH, data={"repeats": "1"}, timeout=30)
    fast_s = time.time() - t0

    # Restore default
    requests.post(f"{device_url}/api/setSpeed", headers=AUTH, data={"speed": "10"}, timeout=5)

    ratio = slow_s / fast_s if fast_s > 0 else 0
    assert ratio >= 5, (
        f"Speed ratio too low: slow={slow_s:.1f}s, fast={fast_s:.1f}s, ratio={ratio:.1f}× "
        f"(expected ≥5×). Servo speed control may not be working."
    )


# ── Schedule ──────────────────────────────────────────────────────────────────

def test_schedule_roundtrip(device_url):
    """Schedule saved to ESP32 must appear in the next status response."""
    new_schedule = json.dumps([{"h": 8, "m": 0, "r": 2}, {"h": 20, "m": 30, "r": 1}])
    requests.post(
        f"{device_url}/api/setFeedTimes",
        headers=AUTH,
        data={"data": new_schedule},
        timeout=5,
    )
    time.sleep(0.3)
    body = requests.get(f"{device_url}/api/status", timeout=5).json()
    times = body.get("feedTimes", [])

    assert len(times) == 2, f"Expected 2 schedule slots, got {len(times)}: {times}"
    h0 = times[0].get("h") or times[0].get("hour")
    assert h0 == 8, f"Expected slot 0 hour=8, got {h0}"


# ── Battery ───────────────────────────────────────────────────────────────────

def test_battery_voltage_rate_limited(device_url):
    """Battery voltage must not change between two requests within 5 seconds.

    Verifies the 5s rate-limit in battery_monitor.cpp is active. If voltage
    fluctuates on every request the device is re-reading ADC every loop cycle
    which wastes CPU and gives noisy readings.
    """
    r1 = requests.get(f"{device_url}/api/status", timeout=5).json()
    time.sleep(1)
    r2 = requests.get(f"{device_url}/api/status", timeout=5).json()
    v1 = r1.get("batteryVoltage")
    v2 = r2.get("batteryVoltage")
    assert v1 == v2, (
        f"Battery voltage changed within 1s: {v1}V → {v2}V. "
        f"The 5s rate-limit in battery_monitor.cpp may not be active."
    )
