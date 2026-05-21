"""Admin-only routes for user management."""

from datetime import date, datetime, timedelta, timezone

from fastapi import APIRouter, HTTPException, status
from fastapi.params import Depends
from pydantic import BaseModel, EmailStr
from supabase import create_client

try:
    from .config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from .dependencies import UserClaims, require_admin
except ImportError:
    from config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from dependencies import UserClaims, require_admin

router = APIRouter(prefix="/api/v1/admin", tags=["admin"])

# In-memory set of device IDs with simulated offline (resets on server restart)
simulated_offline_devices: set[str] = set()


def _service_client():
    if not SUPABASE_SERVICE_KEY:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Service key not configured",
        )
    return create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)


def _is_active(last_activity: str | None) -> bool:
    if not last_activity:
        return False
    try:
        dt = datetime.fromisoformat(last_activity.replace("Z", "+00:00"))
        return (datetime.now(timezone.utc) - dt).days < 7
    except (ValueError, TypeError):
        return False


class AdminUserResponse(BaseModel):
    user_id: str
    email: str
    role: str
    created_at: str
    device_count: int
    last_activity: str | None
    is_active: bool


class AdminDeviceResponse(BaseModel):
    id: str
    name: str
    mac_address: str | None
    created_at: str
    last_seen: str | None


class RoleUpdateRequest(BaseModel):
    role: str


class CreateUserRequest(BaseModel):
    email: EmailStr
    password: str
    role: str = "user"


def _group_by_day(rows: list[dict], key: str, days: int) -> list[int]:
    """Count rows per day for the last N days (UTC), oldest → newest."""
    from collections import defaultdict
    today = date.today()
    counts: dict = defaultdict(int)
    for row in rows:
        ts = row.get(key) or ""
        try:
            dt = datetime.fromisoformat(ts.replace("Z", "+00:00"))
            counts[dt.astimezone(timezone.utc).date()] += 1
        except (ValueError, TypeError):
            pass
    return [counts.get(today - timedelta(days=days - 1 - i), 0) for i in range(days)]


def _group_by_hour(rows: list[dict], key: str) -> list[int]:
    """Count rows per hour for today (UTC), returns list of length = current_hour+1."""
    from collections import defaultdict
    today_utc = date.today()
    counts: dict = defaultdict(int)
    for row in rows:
        ts = row.get(key) or ""
        try:
            dt = datetime.fromisoformat(ts.replace("Z", "+00:00")).astimezone(timezone.utc)
            if dt.date() == today_utc:
                counts[dt.hour] += 1
        except (ValueError, TypeError):
            pass
    now_hour = datetime.now(timezone.utc).hour
    return [counts.get(h, 0) for h in range(now_hour + 1)]


def _group_active_by_day(rows: list[dict], days: int) -> list[int]:
    """Count distinct user_ids per day for the last N days (UTC)."""
    from collections import defaultdict
    today = date.today()
    users_by_day: dict = defaultdict(set)
    for row in rows:
        ts = row.get("created_at") or ""
        uid = row.get("user_id") or ""
        try:
            d = datetime.fromisoformat(ts.replace("Z", "+00:00")).astimezone(timezone.utc).date()
            users_by_day[d].add(uid)
        except (ValueError, TypeError):
            pass
    return [len(users_by_day.get(today - timedelta(days=days - 1 - i), set())) for i in range(days)]


class AdminStatsResponse(BaseModel):
    total_users: int
    total_devices: int
    feedings_today: int
    active_users_week: int
    users_sparkline: list[int]
    devices_sparkline: list[int]
    feedings_sparkline: list[int]
    active_sparkline: list[int]


class AdminFeedEventResponse(BaseModel):
    id: str
    created_at: str
    source: str
    repeats: int
    user_email: str
    device_name: str


def _row_to_user(row: dict) -> AdminUserResponse:
    last_activity = row.get("last_activity")
    return AdminUserResponse(
        user_id=row["id"],
        email=row["email"],
        role=row.get("role", "user"),
        created_at=row["created_at"],
        device_count=int(row.get("device_count", 0)),
        last_activity=last_activity,
        is_active=_is_active(last_activity),
    )


@router.get("/users", response_model=list[AdminUserResponse])
async def list_users(_: UserClaims = Depends(require_admin)):
    client = _service_client()
    result = client.table("admin_users").select("*").order("created_at", desc=True).execute()
    return [_row_to_user(row) for row in result.data]


@router.get("/stats", response_model=AdminStatsResponse)
async def get_stats(_: UserClaims = Depends(require_admin)):
    client = _service_client()

    users_result = client.table("admin_users").select("id", count="exact").execute()
    total_users = users_result.count or 0

    devices_result = client.table("devices").select("id", count="exact").execute()
    total_devices = devices_result.count or 0

    today_start = datetime.combine(date.today(), datetime.min.time(), tzinfo=timezone.utc).isoformat()
    feedings_result = (
        client.table("feed_events")
        .select("id", count="exact")
        .gte("created_at", today_start)
        .execute()
    )
    feedings_today = feedings_result.count or 0

    week_ago = (datetime.now(timezone.utc) - timedelta(days=7)).isoformat()
    active_result = (
        client.table("admin_users")
        .select("id", count="exact")
        .gte("last_activity", week_ago)
        .execute()
    )
    active_users_week = active_result.count or 0

    # Sparkline series (last 7 days)
    sparkline_start = (datetime.now(timezone.utc) - timedelta(days=7)).isoformat()

    users_hist = client.table("admin_users").select("created_at").gte("created_at", sparkline_start).execute()
    users_sparkline = _group_by_day(users_hist.data, "created_at", 7)

    devices_hist = client.table("devices").select("created_at").gte("created_at", sparkline_start).execute()
    devices_sparkline = _group_by_day(devices_hist.data, "created_at", 7)

    feedings_hist = client.table("feed_events").select("created_at").gte("created_at", today_start).execute()
    feedings_sparkline = _group_by_hour(feedings_hist.data, "created_at")

    active_hist = client.table("feed_events").select("created_at, user_id").gte("created_at", sparkline_start).execute()
    active_sparkline = _group_active_by_day(active_hist.data, 7)

    return AdminStatsResponse(
        total_users=total_users,
        total_devices=total_devices,
        feedings_today=feedings_today,
        active_users_week=active_users_week,
        users_sparkline=users_sparkline,
        devices_sparkline=devices_sparkline,
        feedings_sparkline=feedings_sparkline,
        active_sparkline=active_sparkline,
    )


@router.get("/feed-events", response_model=list[AdminFeedEventResponse])
async def list_feed_events(
    limit: int = 50,
    offset: int = 0,
    _: UserClaims = Depends(require_admin),
):
    client = _service_client()
    result = (
        client.table("admin_feed_events")
        .select("id, created_at, source, repeats, user_email, device_name")
        .range(offset, offset + limit - 1)
        .execute()
    )
    return [
        AdminFeedEventResponse(
            id=row["id"],
            created_at=row["created_at"],
            source=row["source"],
            repeats=int(row["repeats"]),
            user_email=row["user_email"],
            device_name=row["device_name"],
        )
        for row in result.data
    ]


@router.post("/users", response_model=AdminUserResponse, status_code=status.HTTP_201_CREATED)
async def create_user(
    request: CreateUserRequest,
    _: UserClaims = Depends(require_admin),
):
    if request.role not in ("admin", "user"):
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Роль має бути 'admin' або 'user'",
        )
    client = _service_client()
    try:
        response = client.auth.admin.create_user({
            "email": request.email,
            "password": request.password,
            "email_confirm": True,
        })
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=str(e))

    if response.user is None:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Не вдалося створити користувача")

    user_id = str(response.user.id)

    # Ensure profile row exists and set the requested role
    client.table("profiles").upsert({"id": user_id, "role": request.role}).execute()

    # Re-fetch from the view for a consistent response
    result = client.table("admin_users").select("*").eq("id", user_id).maybe_single().execute()
    if result.data:
        return _row_to_user(result.data)

    return AdminUserResponse(
        user_id=user_id,
        email=request.email,
        role=request.role,
        created_at=str(response.user.created_at),
        device_count=0,
        last_activity=None,
        is_active=False,
    )


@router.patch("/users/{user_id}/role", response_model=AdminUserResponse)
async def update_user_role(
    user_id: str,
    request: RoleUpdateRequest,
    current_admin: UserClaims = Depends(require_admin),
):
    if request.role not in ("admin", "user"):
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Роль має бути 'admin' або 'user'",
        )
    if user_id == current_admin.id and request.role != "admin":
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Не можна змінити власну роль",
        )
    client = _service_client()
    client.table("profiles").update({"role": request.role}).eq("id", user_id).execute()
    result = client.table("admin_users").select("*").eq("id", user_id).maybe_single().execute()
    if not result.data:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Користувача не знайдено")
    return _row_to_user(result.data)


@router.delete("/users/{user_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_user(
    user_id: str,
    current_admin: UserClaims = Depends(require_admin),
):
    if user_id == current_admin.id:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Не можна видалити власний акаунт",
        )
    client = _service_client()
    try:
        client.auth.admin.delete_user(user_id)
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=str(e))


@router.get("/users/{user_id}/devices", response_model=list[AdminDeviceResponse])
async def list_user_devices(
    user_id: str,
    _: UserClaims = Depends(require_admin),
):
    client = _service_client()
    result = (
        client.table("devices")
        .select("id, name, mac_address, created_at, last_seen")
        .eq("user_id", user_id)
        .order("created_at", desc=True)
        .execute()
    )
    return [
        AdminDeviceResponse(
            id=row["id"],
            name=row["name"],
            mac_address=row.get("mac_address"),
            created_at=row["created_at"],
            last_seen=row.get("last_seen"),
        )
        for row in result.data
    ]


@router.delete(
    "/users/{user_id}/devices/{device_id}",
    status_code=status.HTTP_204_NO_CONTENT,
)
async def delete_user_device(
    user_id: str,
    device_id: str,
    _: UserClaims = Depends(require_admin),
):
    client = _service_client()
    result = (
        client.table("devices")
        .delete()
        .eq("id", device_id)
        .eq("user_id", user_id)
        .execute()
    )
    if not result.data:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Пристрій не знайдено")


@router.get("/devices/sim-offline", response_model=list[str])
async def get_simulated_offline(_: UserClaims = Depends(require_admin)):
    return list(simulated_offline_devices)


@router.post("/devices/{device_id}/sim-offline", status_code=status.HTTP_204_NO_CONTENT)
async def set_simulated_offline(device_id: str, _: UserClaims = Depends(require_admin)):
    simulated_offline_devices.add(device_id)


@router.delete("/devices/{device_id}/sim-offline", status_code=status.HTTP_204_NO_CONTENT)
async def clear_simulated_offline(device_id: str, _: UserClaims = Depends(require_admin)):
    simulated_offline_devices.discard(device_id)
