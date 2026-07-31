"""Auth endpoint tests — patch Supabase client, no hardware/real Supabase needed.

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
    AUTH_MODULE = "backend.auth"
except ImportError:
    from main import app
    from dependencies import UserClaims, get_current_user
    AUTH_MODULE = "auth"

GET_SUPABASE_TARGET = f"{AUTH_MODULE}.get_supabase"
FETCH_ROLE_TARGET = f"{AUTH_MODULE}._fetch_role"

pytestmark = pytest.mark.mock

client = TestClient(app)


def _fake_session_response(user_id="user-1", email="a@example.com"):
    user = MagicMock(id=user_id, email=email, created_at=None)
    session = MagicMock(access_token="access-tok", refresh_token="refresh-tok")
    return MagicMock(user=user, session=session)


# ── register ──────────────────────────────────────────────────────────────────

def test_register_success() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_up.return_value = _fake_session_response()

    with patch(GET_SUPABASE_TARGET, return_value=fake_client), \
         patch(FETCH_ROLE_TARGET, return_value="user"):
        response = client.post("/auth/register", json={"email": "a@example.com", "password": "hunter22"})

    assert response.status_code == 201
    body = response.json()
    assert body["access_token"] == "access-tok"
    assert body["role"] == "user"


def test_register_fails_when_supabase_raises() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_up.side_effect = RuntimeError("email already registered")

    with patch(GET_SUPABASE_TARGET, return_value=fake_client):
        response = client.post("/auth/register", json={"email": "a@example.com", "password": "hunter22"})

    assert response.status_code == 400


def test_register_fails_when_session_missing() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_up.return_value = MagicMock(user=None, session=None)

    with patch(GET_SUPABASE_TARGET, return_value=fake_client):
        response = client.post("/auth/register", json={"email": "a@example.com", "password": "hunter22"})

    assert response.status_code == 400


# ── login ─────────────────────────────────────────────────────────────────────

def test_login_success() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_in_with_password.return_value = _fake_session_response()

    with patch(GET_SUPABASE_TARGET, return_value=fake_client), \
         patch(FETCH_ROLE_TARGET, return_value="admin"):
        response = client.post("/auth/login", json={"email": "a@example.com", "password": "hunter22"})

    assert response.status_code == 200
    assert response.json()["role"] == "admin"


def test_login_rejects_wrong_password() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_in_with_password.side_effect = RuntimeError("Invalid login credentials")

    with patch(GET_SUPABASE_TARGET, return_value=fake_client):
        response = client.post("/auth/login", json={"email": "a@example.com", "password": "wrong"})

    assert response.status_code == 401


def test_login_rejects_when_no_session_returned() -> None:
    fake_client = MagicMock()
    fake_client.auth.sign_in_with_password.return_value = MagicMock(user=None, session=None)

    with patch(GET_SUPABASE_TARGET, return_value=fake_client):
        response = client.post("/auth/login", json={"email": "a@example.com", "password": "hunter22"})

    assert response.status_code == 401


# ── refresh ───────────────────────────────────────────────────────────────────

def test_refresh_success() -> None:
    fake_client = MagicMock()
    fake_client.auth.refresh_session.return_value = _fake_session_response()

    with patch(GET_SUPABASE_TARGET, return_value=fake_client), \
         patch(FETCH_ROLE_TARGET, return_value="user"):
        response = client.post("/auth/refresh", json={"refresh_token": "old-refresh"})

    assert response.status_code == 200
    assert response.json()["access_token"] == "access-tok"


def test_refresh_rejects_invalid_token() -> None:
    fake_client = MagicMock()
    fake_client.auth.refresh_session.side_effect = RuntimeError("invalid refresh token")

    with patch(GET_SUPABASE_TARGET, return_value=fake_client):
        response = client.post("/auth/refresh", json={"refresh_token": "garbage"})

    assert response.status_code == 401


# ── me / logout (require bearer auth) ──────────────────────────────────────────

def test_me_requires_authorization_header() -> None:
    # test_main_endpoints.py installs a permanent fake-user override for the
    # whole session; restore the real dependency here so the "no header"
    # case is actually exercised instead of silently authenticating.
    with override_dependency(app, get_current_user, get_current_user):
        response = client.get("/auth/me")
    assert response.status_code in (401, 403)


def test_me_returns_current_user_when_authenticated() -> None:
    fake_claims = UserClaims(id="user-1", email="a@example.com", role="admin")
    with override_dependency(app, get_current_user, lambda: fake_claims):
        response = client.get("/auth/me", headers={"Authorization": "Bearer sometoken"})

    assert response.status_code == 200
    body = response.json()
    assert body == {"user_id": "user-1", "email": "a@example.com", "role": "admin"}


def test_logout_requires_authorization_header() -> None:
    with override_dependency(app, get_current_user, get_current_user):
        response = client.post("/auth/logout")
    assert response.status_code in (401, 403)
