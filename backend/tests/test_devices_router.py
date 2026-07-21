"""HTTP-level tests for the /api/v1/devices router — DeviceService swapped for
an in-memory fake, no real Supabase touched.

Run with: pytest -m mock
"""
from __future__ import annotations

import pytest
from fastapi.testclient import TestClient

from conftest import override_dependency

try:
    from backend.main import app
    from backend.dependencies import UserClaims, get_current_user
    from backend.devices import get_device_service
    from backend.device_service import InMemoryDeviceService
except ImportError:
    from main import app
    from dependencies import UserClaims, get_current_user
    from devices import get_device_service
    from device_service import InMemoryDeviceService

pytestmark = pytest.mark.mock

client = TestClient(app)

USER_A = UserClaims(id="user-a", email="a@example.com", role="user")
USER_B = UserClaims(id="user-b", email="b@example.com", role="user")


@pytest.fixture
def fake_device_service():
    """Fresh in-memory device store per test, bypassing Supabase entirely."""
    service = InMemoryDeviceService()
    with override_dependency(app, get_device_service, lambda: service):
        yield service


def test_devices_endpoints_require_authorization_header(fake_device_service) -> None:
    # test_main_endpoints.py installs a permanent fake-user override for the
    # whole session; restore the real dependency here so the "no header"
    # case is actually exercised instead of silently authenticating.
    with override_dependency(app, get_current_user, get_current_user):
        response = client.get("/api/v1/devices")
    assert response.status_code in (401, 403)


def test_create_and_list_device_for_authenticated_user(fake_device_service) -> None:
    with override_dependency(app, get_current_user, lambda: USER_A):
        create_resp = client.post(
            "/api/v1/devices",
            json={"name": "Вітальня"},
            headers={"Authorization": "Bearer tok"},
        )
        list_resp = client.get("/api/v1/devices", headers={"Authorization": "Bearer tok"})

    assert create_resp.status_code == 201
    assert create_resp.json()["name"] == "Вітальня"
    assert [d["name"] for d in list_resp.json()] == ["Вітальня"]


def test_user_cannot_access_another_users_device(fake_device_service) -> None:
    with override_dependency(app, get_current_user, lambda: USER_A):
        created = client.post(
            "/api/v1/devices", json={"name": "Спальня"}, headers={"Authorization": "Bearer tok"}
        ).json()

    with override_dependency(app, get_current_user, lambda: USER_B):
        response = client.get(f"/api/v1/devices/{created['id']}", headers={"Authorization": "Bearer tok"})

    assert response.status_code == 404


def test_rename_device_conflict_returns_409(fake_device_service) -> None:
    with override_dependency(app, get_current_user, lambda: USER_A):
        client.post("/api/v1/devices", json={"name": "Офіс"}, headers={"Authorization": "Bearer tok"})
        second = client.post(
            "/api/v1/devices", json={"name": "Кухня"}, headers={"Authorization": "Bearer tok"}
        ).json()
        response = client.patch(
            f"/api/v1/devices/{second['id']}",
            json={"name": "Офіс"},
            headers={"Authorization": "Bearer tok"},
        )

    assert response.status_code == 409


def test_delete_device_removes_it_from_list(fake_device_service) -> None:
    with override_dependency(app, get_current_user, lambda: USER_A):
        created = client.post(
            "/api/v1/devices", json={"name": "Тераса"}, headers={"Authorization": "Bearer tok"}
        ).json()
        delete_resp = client.delete(f"/api/v1/devices/{created['id']}", headers={"Authorization": "Bearer tok"})
        list_resp = client.get("/api/v1/devices", headers={"Authorization": "Bearer tok"})

    assert delete_resp.status_code == 204
    assert list_resp.json() == []
