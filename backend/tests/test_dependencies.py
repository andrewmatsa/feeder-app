"""Auth dependency unit tests — no HTTP, no real Supabase.

Run with: pytest -m mock
"""
from __future__ import annotations

import asyncio
from unittest.mock import MagicMock, patch

import pytest
from fastapi import HTTPException
from fastapi.security import HTTPAuthorizationCredentials

try:
    from backend.dependencies import UserClaims, _fetch_role, get_current_user, require_admin
    MODULE = "backend.dependencies"
except ImportError:
    from dependencies import UserClaims, _fetch_role, get_current_user, require_admin
    MODULE = "dependencies"

CREATE_CLIENT_TARGET = f"{MODULE}.create_client"
SERVICE_KEY_TARGET = f"{MODULE}.SUPABASE_SERVICE_KEY"

pytestmark = pytest.mark.mock


def run(coro):
    return asyncio.run(coro)


def _creds(token: str = "sometoken") -> HTTPAuthorizationCredentials:
    return HTTPAuthorizationCredentials(scheme="Bearer", credentials=token)


# ── get_current_user ─────────────────────────────────────────────────────────

def test_get_current_user_returns_claims_for_valid_token() -> None:
    fake_user = MagicMock(id="user-1", email="a@example.com")
    fake_client = MagicMock()
    fake_client.auth.get_user.return_value = MagicMock(user=fake_user)

    with patch(CREATE_CLIENT_TARGET, return_value=fake_client), \
         patch(f"{MODULE}._fetch_role", return_value="user"):
        claims = run(get_current_user(_creds()))

    assert claims == UserClaims(id="user-1", email="a@example.com", role="user")


def test_get_current_user_rejects_when_supabase_returns_no_user() -> None:
    fake_client = MagicMock()
    fake_client.auth.get_user.return_value = MagicMock(user=None)

    with patch(CREATE_CLIENT_TARGET, return_value=fake_client):
        with pytest.raises(HTTPException) as exc:
            run(get_current_user(_creds()))

    assert exc.value.status_code == 401


def test_get_current_user_rejects_on_supabase_exception() -> None:
    with patch(CREATE_CLIENT_TARGET, side_effect=RuntimeError("network down")):
        with pytest.raises(HTTPException) as exc:
            run(get_current_user(_creds("bad-token")))

    assert exc.value.status_code == 401


# ── require_admin ────────────────────────────────────────────────────────────

def test_require_admin_allows_admin_role() -> None:
    admin_claims = UserClaims(id="u1", email="a@example.com", role="admin")
    result = run(require_admin(admin_claims))
    assert result is admin_claims


def test_require_admin_blocks_non_admin_role() -> None:
    user_claims = UserClaims(id="u1", email="a@example.com", role="user")
    with pytest.raises(HTTPException) as exc:
        run(require_admin(user_claims))
    assert exc.value.status_code == 403


# ── _fetch_role ───────────────────────────────────────────────────────────────

def test_fetch_role_returns_user_when_service_key_missing() -> None:
    with patch(SERVICE_KEY_TARGET, ""):
        assert _fetch_role("user-1") == "user"


def test_fetch_role_returns_role_from_profiles_table() -> None:
    fake_result = MagicMock(data={"role": "admin"})
    fake_client = MagicMock()
    fake_client.table.return_value.select.return_value.eq.return_value.maybe_single.return_value.execute.return_value = fake_result

    with patch(SERVICE_KEY_TARGET, "svc-key"), patch(CREATE_CLIENT_TARGET, return_value=fake_client):
        assert _fetch_role("user-1") == "admin"


def test_fetch_role_falls_back_to_user_on_exception() -> None:
    with patch(SERVICE_KEY_TARGET, "svc-key"), patch(CREATE_CLIENT_TARGET, side_effect=RuntimeError("down")):
        assert _fetch_role("user-1") == "user"
