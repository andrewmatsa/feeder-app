"""FastAPI dependencies for authentication."""

from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from pydantic import BaseModel
from supabase import create_client

try:
    from .config import SUPABASE_ANON_KEY, SUPABASE_SERVICE_KEY, SUPABASE_URL
except ImportError:
    from config import SUPABASE_ANON_KEY, SUPABASE_SERVICE_KEY, SUPABASE_URL

bearer_scheme = HTTPBearer()


class UserClaims(BaseModel):
    id: str
    email: str
    role: str = "user"


async def get_bearer_token(
    credentials: HTTPAuthorizationCredentials = Depends(bearer_scheme),
) -> str:
    return credentials.credentials


def _fetch_role(user_id: str) -> str:
    """Fetch role from profiles table via service-role key (bypasses RLS)."""
    if not SUPABASE_SERVICE_KEY:
        return "user"
    try:
        svc = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
        result = (
            svc.table("profiles")
            .select("role")
            .eq("id", user_id)
            .maybe_single()
            .execute()
        )
        if result.data:
            return result.data.get("role", "user")
    except Exception:
        pass
    return "user"


async def get_current_user(
    credentials: HTTPAuthorizationCredentials = Depends(bearer_scheme),
) -> UserClaims:
    token = credentials.credentials
    try:
        client = create_client(SUPABASE_URL, SUPABASE_ANON_KEY)
        response = client.auth.get_user(token)
        if response.user is None:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid token",
                headers={"WWW-Authenticate": "Bearer"},
            )
        user_id = str(response.user.id)
        email = response.user.email or ""
        role = _fetch_role(user_id)
        return UserClaims(id=user_id, email=email, role=role)
    except HTTPException:
        raise
    except Exception:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid or expired token",
            headers={"WWW-Authenticate": "Bearer"},
        )


async def require_admin(
    current_user: UserClaims = Depends(get_current_user),
) -> UserClaims:
    if current_user.role != "admin":
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Потрібні права адміністратора",
        )
    return current_user
