"""Firmware contract tests — patch request_firmware, no hardware needed.

These tests verify that each API endpoint calls the firmware with the correct
payload and correctly propagates errors. MOCK_DEVICE is forced off so the real
request path is exercised.

Run with: pytest -m mock
"""
from __future__ import annotations

import json
from unittest.mock import AsyncMock, Mock, patch

import pytest
from fastapi import HTTPException
from fastapi.testclient import TestClient

pytestmark = pytest.mark.mock

try:
    from backend.main import app
    import backend.main as _main
    from backend.dependencies import UserClaims, get_current_user
    REQUEST_FIRMWARE_TARGET = "backend.main.request_firmware"
except ImportError:
    from main import app
    import main as _main
    from dependencies import UserClaims, get_current_user
    REQUEST_FIRMWARE_TARGET = "main.request_firmware"


def _fake_user() -> UserClaims:
    return UserClaims(id="test-user-id", email="test@example.com", role="user")


app.dependency_overrides[get_current_user] = _fake_user
client = TestClient(app)


@pytest.fixture(autouse=True)
def disable_mock_device():
    """Force MOCK_DEVICE=False so real firmware path is taken."""
    with patch.object(_main, "MOCK_DEVICE", False):
        yield


# ── Feed ──────────────────────────────────────────────────────────────────────

def test_feed_endpoint_calls_firmware_with_repeats() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.return_value = None

        response = client.post("/api/feed", json={"repeats": 2})

        assert response.status_code == 200
        body = response.json()
        assert body["success"] is True
        assert "2" in body["message"]

        args, kwargs = mock_fw.await_args
        assert args[0] == "/api/feedNow"
        assert kwargs["method"] == "POST"
        assert kwargs["data"] == {"repeats": 2}


def test_feed_endpoint_validation_error_for_invalid_repeats() -> None:
    response = client.post("/api/feed", json={"repeats": 0})
    assert response.status_code == 422


def test_feed_endpoint_propagates_firmware_timeout_as_504() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.side_effect = HTTPException(status_code=504, detail="ESP32 request timed out")

        response = client.post("/api/feed", json={"repeats": 2})

        assert response.status_code == 504
        assert "timed out" in response.json()["detail"]


# ── Status ────────────────────────────────────────────────────────────────────

def test_status_endpoint_maps_payload_from_firmware() -> None:
    firmware_payload = {
        "currentAngle": 45,
        "speed": 20,
        "feedRepeats": 2,
        "batteryVoltage": 7.7,
        "batteryPercent": 88,
        "isCharging": False,
        "feedTimes": [{"h": 10, "m": 0, "r": 1}],
    }
    fake_response = Mock()
    fake_response.json.return_value = firmware_payload

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.return_value = fake_response

        response = client.get("/api/status")

        assert response.status_code == 200
        body = response.json()
        assert body["angle"] == 45
        assert body["batteryPercent"] == 88
        assert len(body["feedTimes"]) == 1
        assert body["feedTimes"][0]["hour"] == 10

        args, _ = mock_fw.await_args
        assert args[0] == "/api/status"


def test_status_endpoint_propagates_firmware_unreachable_as_503() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.side_effect = HTTPException(status_code=503, detail="Failed to communicate with ESP32")

        response = client.get("/api/status")

        assert response.status_code == 503
        assert "Failed to communicate" in response.json()["detail"]


# ── Schedule ──────────────────────────────────────────────────────────────────

def test_schedule_endpoint_calls_firmware_with_encoded_payload() -> None:
    payload = {
        "times": [
            {"hour": 10, "minute": 0, "repeats": 2},
            {"hour": 18, "minute": 30, "repeats": 1},
        ]
    }
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.return_value = None

        response = client.post("/api/schedule", json=payload)

        assert response.status_code == 200
        mock_fw.assert_awaited_once()
        args, kwargs = mock_fw.await_args
        assert args[0] == "/api/setFeedTimes"
        assert kwargs["method"] == "POST"
        assert kwargs["data"]["data"] == '[{"h":10,"m":0,"r":2},{"h":18,"m":30,"r":1}]'


# ── Power mode ────────────────────────────────────────────────────────────────

def test_power_mode_endpoint_calls_firmware_with_string_bool() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.return_value = None

        response = client.post("/api/power-mode", json={"enabled": True})

        assert response.status_code == 200
        args, kwargs = mock_fw.await_args
        assert args[0] == "/api/setPowerMode"
        assert kwargs["method"] == "POST"
        assert kwargs["data"] == {"enabled": "true"}


def test_power_mode_endpoint_validation_error_when_enabled_missing() -> None:
    response = client.post("/api/power-mode", json={})
    assert response.status_code == 422


def test_power_mode_endpoint_propagates_firmware_502() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.side_effect = HTTPException(status_code=502, detail="ESP32 rejected the request")

        response = client.post("/api/power-mode", json={"enabled": False})

        assert response.status_code == 502
        assert "rejected" in response.json()["detail"]


# ── Flow tests ────────────────────────────────────────────────────────────────

def test_schedule_persists_and_is_returned_by_status_flow() -> None:
    firmware_state: dict = {"feedTimes": []}

    async def firmware_mock(path: str, **kwargs):
        if path == "/api/setFeedTimes":
            firmware_state["feedTimes"] = json.loads(kwargs.get("data", {}).get("data", "[]"))
            return None
        if path == "/api/status":
            fake = Mock()
            fake.json.return_value = {
                "currentAngle": 0, "speed": 20, "feedRepeats": 1,
                "batteryVoltage": 7.6, "batteryPercent": 70, "isCharging": False,
                "feedTimes": firmware_state["feedTimes"],
            }
            return fake
        raise AssertionError(f"Unexpected firmware path: {path}")

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.side_effect = firmware_mock

        r = client.post("/api/schedule", json={"times": [
            {"hour": 9, "minute": 15, "repeats": 2},
            {"hour": 21, "minute": 45, "repeats": 1},
        ]})
        assert r.status_code == 200

        body = client.get("/api/status").json()
        assert body["feedTimes"] == [
            {"hour": 9, "minute": 15, "repeats": 2},
            {"hour": 21, "minute": 45, "repeats": 1},
        ]


def test_set_endpoints_persist_core_fields_and_are_reflected_in_status() -> None:
    firmware_state: dict = {
        "currentAngle": 0, "speed": 20, "feedRepeats": 1,
        "powerSaveMode": True, "displayEnabled": True,
        "displayOffAfterSec": 20, "deepSleepIdleSec": 60,
        "batteryVoltage": 7.5, "batteryPercent": 65,
        "isCharging": False, "feedTimes": [],
    }

    async def firmware_mock(path: str, **kwargs):
        data = kwargs.get("data", {})
        if path == "/api/setAngle":
            firmware_state["currentAngle"] = int(data.get("angle", 0))
        elif path == "/api/setSpeed":
            firmware_state["speed"] = int(data.get("speed", 0))
        elif path == "/api/setRepeats":
            firmware_state["feedRepeats"] = int(data.get("repeats", 1))
        elif path == "/api/setPowerMode":
            firmware_state["powerSaveMode"] = str(data.get("enabled", "false")).lower() == "true"
        elif path == "/api/feedNow":
            pass
        elif path == "/api/setFeedTimes":
            firmware_state["feedTimes"] = json.loads(data.get("data", "[]"))
        elif path == "/api/status":
            fake = Mock()
            fake.json.return_value = firmware_state
            return fake
        else:
            raise AssertionError(f"Unexpected firmware path: {path}")
        return None

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mock_fw:
        mock_fw.side_effect = firmware_mock

        assert client.post("/api/angle", json={"angle": 33}).status_code == 200
        assert client.post("/api/speed", json={"speed": 12}).status_code == 200
        assert client.post("/api/feed", json={"repeats": 4}).status_code == 200
        assert client.post("/api/power-mode", json={"enabled": False}).status_code == 200
        assert client.post("/api/schedule", json={"times": [{"hour": 6, "minute": 45, "repeats": 2}]}).status_code == 200

        body = client.get("/api/status").json()
        assert body["angle"] == 33
        assert body["speed"] == 12
        assert body["feedRepeats"] == 1  # /api/feed is one-shot, doesn't change status
        assert body["powerSaveMode"] is False
        assert body["feedTimes"] == [{"hour": 6, "minute": 45, "repeats": 2}]
