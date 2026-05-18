from __future__ import annotations

import json
from unittest.mock import AsyncMock, Mock, patch

from fastapi import HTTPException
from fastapi.testclient import TestClient

try:
    from backend.main import app
    REQUEST_FIRMWARE_TARGET = "backend.main.request_firmware"
except ImportError:
    from main import app
    REQUEST_FIRMWARE_TARGET = "main.request_firmware"


client = TestClient(app)


def test_feed_endpoint_calls_firmware_with_repeats() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.return_value = None

        response = client.post("/api/feed", json={"repeats": 2})

        assert response.status_code == 200
        body = response.json()
        assert body["success"] is True
        assert "2" in body["message"]

        mocked_request.assert_awaited_once_with(
            "/api/feedNow",
            method="POST",
            data={"repeats": 2},
        )


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

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.return_value = fake_response

        response = client.get("/api/status")

        assert response.status_code == 200
        body = response.json()
        assert body["angle"] == 45
        assert body["batteryPercent"] == 88
        assert len(body["feedTimes"]) == 1
        assert body["feedTimes"][0]["hour"] == 10

        mocked_request.assert_awaited_once_with("/api/status")


def test_feed_endpoint_validation_error_for_invalid_repeats() -> None:
    response = client.post("/api/feed", json={"repeats": 0})
    assert response.status_code == 422


def test_schedule_endpoint_calls_firmware_with_encoded_payload() -> None:
    payload = {
        "times": [
            {"hour": 10, "minute": 0, "repeats": 2},
            {"hour": 18, "minute": 30, "repeats": 1},
        ]
    }
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.return_value = None

        response = client.post("/api/schedule", json=payload)

        assert response.status_code == 200
        mocked_request.assert_awaited_once()
        args, kwargs = mocked_request.await_args
        assert args[0] == "/api/setFeedTimes"
        assert kwargs["method"] == "POST"
        assert "data" in kwargs["data"]
        # compact format produced by encode_schedule
        assert kwargs["data"]["data"] == '[{"h":10,"m":0,"r":2},{"h":18,"m":30,"r":1}]'


def test_feed_endpoint_propagates_firmware_timeout_as_504() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.side_effect = HTTPException(status_code=504, detail="ESP32 request timed out")

        response = client.post("/api/feed", json={"repeats": 2})

        assert response.status_code == 504
        assert response.json()["detail"] == "ESP32 request timed out"


def test_status_endpoint_propagates_firmware_unreachable_as_503() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.side_effect = HTTPException(status_code=503, detail="Failed to communicate with ESP32")

        response = client.get("/api/status")

        assert response.status_code == 503
        assert "Failed to communicate with ESP32" in response.json()["detail"]


def test_power_mode_endpoint_calls_firmware_with_string_bool() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.return_value = None

        response = client.post("/api/power-mode", json={"enabled": True})

        assert response.status_code == 200
        mocked_request.assert_awaited_once_with(
            "/api/setPowerMode",
            method="POST",
            data={"enabled": "true"},
        )


def test_power_mode_endpoint_validation_error_when_enabled_missing() -> None:
    response = client.post("/api/power-mode", json={})
    assert response.status_code == 422


def test_power_mode_endpoint_propagates_firmware_502() -> None:
    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.side_effect = HTTPException(status_code=502, detail="ESP32 rejected the request")

        response = client.post("/api/power-mode", json={"enabled": False})

        assert response.status_code == 502
        assert "ESP32 rejected the request" in response.json()["detail"]


def test_schedule_persists_and_is_returned_by_status_flow() -> None:
    # Simulate firmware-side persistence in memory for set->status flow.
    firmware_state = {"feedTimes": []}

    async def firmware_mock(path: str, **kwargs):
        if path == "/api/setFeedTimes":
            encoded = kwargs.get("data", {}).get("data", "[]")
            firmware_state["feedTimes"] = json.loads(encoded)
            return None

        if path == "/api/status":
            fake_response = Mock()
            fake_response.json.return_value = {
                "currentAngle": 0,
                "speed": 20,
                "feedRepeats": 1,
                "batteryVoltage": 7.6,
                "batteryPercent": 70,
                "isCharging": False,
                "feedTimes": firmware_state["feedTimes"],
            }
            return fake_response

        raise AssertionError(f"Unexpected firmware path: {path}")

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.side_effect = firmware_mock

        new_schedule = {
            "times": [
                {"hour": 9, "minute": 15, "repeats": 2},
                {"hour": 21, "minute": 45, "repeats": 1},
            ]
        }

        set_response = client.post("/api/schedule", json=new_schedule)
        assert set_response.status_code == 200

        status_response = client.get("/api/status")
        assert status_response.status_code == 200
        body = status_response.json()
        assert body["feedTimes"] == [
            {"hour": 9, "minute": 15, "repeats": 2},
            {"hour": 21, "minute": 45, "repeats": 1},
        ]


def test_set_endpoints_persist_core_fields_and_are_reflected_in_status() -> None:
    # Simulate firmware state mutations from all "set" endpoints.
    firmware_state = {
        "currentAngle": 0,
        "speed": 20,
        "feedRepeats": 1,
        "powerSaveMode": True,
        "displayEnabled": True,
        "displayOffAfterSec": 20,
        "deepSleepIdleSec": 60,
        "batteryVoltage": 7.5,
        "batteryPercent": 65,
        "isCharging": False,
        "feedTimes": [],
    }

    async def firmware_mock(path: str, **kwargs):
        data = kwargs.get("data", {})

        if path == "/api/setAngle":
            firmware_state["currentAngle"] = int(data.get("angle", 0))
            return None
        if path == "/api/setSpeed":
            firmware_state["speed"] = int(data.get("speed", 0))
            return None
        if path == "/api/setRepeats":
            firmware_state["feedRepeats"] = int(data.get("repeats", 1))
            return None
        if path == "/api/setPowerMode":
            firmware_state["powerSaveMode"] = str(data.get("enabled", "false")).lower() == "true"
            return None
        if path == "/api/feedNow":
            # One-shot action endpoint: does not persist feedRepeats backend setting.
            return None
        if path == "/api/setFeedTimes":
            firmware_state["feedTimes"] = json.loads(data.get("data", "[]"))
            return None
        if path == "/api/status":
            fake_response = Mock()
            fake_response.json.return_value = firmware_state
            return fake_response

        raise AssertionError(f"Unexpected firmware path: {path}")

    with patch(REQUEST_FIRMWARE_TARGET, new_callable=AsyncMock) as mocked_request:
        mocked_request.side_effect = firmware_mock

        assert client.post("/api/angle", json={"angle": 33}).status_code == 200
        assert client.post("/api/speed", json={"speed": 12}).status_code == 200
        assert client.post("/api/feed", json={"repeats": 4}).status_code == 200
        assert client.post("/api/power-mode", json={"enabled": False}).status_code == 200
        assert (
            client.post(
                "/api/schedule",
                json={"times": [{"hour": 6, "minute": 45, "repeats": 2}]},
            ).status_code
            == 200
        )

        status_response = client.get("/api/status")
        assert status_response.status_code == 200
        body = status_response.json()
        assert body["angle"] == 33
        assert body["speed"] == 12
        # /api/feed is one-shot and doesn't persist feedRepeats in status.
        assert body["feedRepeats"] == 1
        assert body["powerSaveMode"] is False
        assert body["feedTimes"] == [{"hour": 6, "minute": 45, "repeats": 2}]
