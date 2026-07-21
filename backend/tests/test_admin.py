"""Admin router tests — role-guard enforcement + representative endpoints,
Supabase service client mocked so no real infra is touched.

Run with: pytest -m mock
"""
from __future__ import annotations

from unittest.mock import MagicMock, patch

import pytest
from fastapi.testclient import TestClient

from conftest import override_dependency

try:
    from backend.main import app
    from backend.dependencies import UserClaims, get_current_user
    ADMIN_MODULE = "backend.admin"
except ImportError:
    from main import app
    from dependencies import UserClaims, get_current_user
    ADMIN_MODULE = "admin"

SERVICE_CLIENT_TARGET = f"{ADMIN_MODULE}._service_client"

pytestmark = pytest.mark.mock

client = TestClient(app)

ADMIN_ENDPOINTS = [
    ("GET", "/api/v1/admin/users"),
    ("GET", "/api/v1/admin/stats"),
    ("GET", "/api/v1/admin/feed-events"),
    ("GET", "/api/v1/admin/devices/sim-offline"),
    ("GET", "/api/v1/admin/users/some-user-id/devices"),
]


def _user(role: str) -> UserClaims:
    return UserClaims(id="caller-id", email="caller@example.com", role=role)


# ── role guard: no auth at all ────────────────────────────────────────────────

@pytest.mark.parametrize("method,path", ADMIN_ENDPOINTS)
def test_admin_endpoints_require_authorization_header(method: str, path: str) -> None:
    # test_main_endpoints.py installs a permanent fake-user override for the
    # whole session; restore the real dependency here so the "no header"
    # case is actually exercised instead of relying on that fake user's role.
    with override_dependency(app, get_current_user, get_current_user):
        response = client.request(method, path)
    assert response.status_code in (401, 403)


# ── role guard: authenticated but not admin ──────────────────────────────────

@pytest.mark.parametrize("method,path", ADMIN_ENDPOINTS)
def test_admin_endpoints_block_non_admin_user(method: str, path: str) -> None:
    with override_dependency(app, get_current_user, lambda: _user("user")):
        response = client.request(method, path, headers={"Authorization": "Bearer sometoken"})

    assert response.status_code == 403


# ── happy path: admin role, Supabase mocked ──────────────────────────────────

def test_list_users_returns_mapped_rows_for_admin() -> None:
    fake_client = MagicMock()
    fake_client.table.return_value.select.return_value.order.return_value.execute.return_value = MagicMock(
        data=[{
            "id": "u1",
            "email": "a@example.com",
            "role": "user",
            "created_at": "2026-01-01T00:00:00Z",
            "device_count": 2,
            "last_activity": None,
        }]
    )

    with override_dependency(app, get_current_user, lambda: _user("admin")), \
         patch(SERVICE_CLIENT_TARGET, return_value=fake_client):
        response = client.get("/api/v1/admin/users", headers={"Authorization": "Bearer sometoken"})

    assert response.status_code == 200
    body = response.json()
    assert body == [{
        "user_id": "u1",
        "email": "a@example.com",
        "role": "user",
        "created_at": "2026-01-01T00:00:00Z",
        "device_count": 2,
        "last_activity": None,
        "is_active": False,
    }]


def test_sim_offline_toggle_roundtrip_for_admin() -> None:
    with override_dependency(app, get_current_user, lambda: _user("admin")):
        set_resp = client.post(
            "/api/v1/admin/devices/device-1/sim-offline",
            headers={"Authorization": "Bearer sometoken"},
        )
        list_resp = client.get(
            "/api/v1/admin/devices/sim-offline",
            headers={"Authorization": "Bearer sometoken"},
        )
        clear_resp = client.delete(
            "/api/v1/admin/devices/device-1/sim-offline",
            headers={"Authorization": "Bearer sometoken"},
        )

    assert set_resp.status_code == 204
    assert "device-1" in list_resp.json()
    assert clear_resp.status_code == 204


def test_cannot_change_own_role() -> None:
    with override_dependency(app, get_current_user, lambda: _user("admin")):
        response = client.patch(
            "/api/v1/admin/users/caller-id/role",
            json={"role": "user"},
            headers={"Authorization": "Bearer sometoken"},
        )

    assert response.status_code == 400


def test_cannot_delete_own_account() -> None:
    with override_dependency(app, get_current_user, lambda: _user("admin")):
        response = client.delete(
            "/api/v1/admin/users/caller-id",
            headers={"Authorization": "Bearer sometoken"},
        )

    assert response.status_code == 400
