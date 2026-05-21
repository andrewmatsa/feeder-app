"""User-owned aquarium devices (display names for multiple feeders per account)."""

from fastapi import APIRouter, Depends, HTTPException, status
from pydantic import BaseModel, Field

try:
    from .config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from .dependencies import UserClaims, get_bearer_token, get_current_user
    from .device_service import DeviceRecord, DeviceService, build_device_service
    from .admin import simulated_offline_devices
except ImportError:
    from config import SUPABASE_SERVICE_KEY, SUPABASE_URL
    from dependencies import UserClaims, get_bearer_token, get_current_user
    from device_service import DeviceRecord, DeviceService, build_device_service
    from admin import simulated_offline_devices

router = APIRouter(prefix="/api/v1/devices", tags=["devices"])


class DeviceResponse(BaseModel):
    id: str
    name: str
    macAddress: str | None = None
    createdAt: str
    lastSeen: str | None = None


class CreateDeviceRequest(BaseModel):
    name: str | None = Field(
        default=None,
        max_length=40,
        description="Display name, e.g. Вітальня or Спальня",
    )
    macAddress: str | None = Field(default=None, description="Hardware MAC after pairing")


class UpdateDeviceRequest(BaseModel):
    name: str | None = Field(default=None, min_length=2, max_length=40)


def _to_response(record: DeviceRecord) -> DeviceResponse:
    last_seen = None if record.id in simulated_offline_devices else (
        record.last_seen.isoformat() if record.last_seen else None
    )
    return DeviceResponse(
        id=record.id,
        name=record.name,
        macAddress=record.mac_address,
        createdAt=record.created_at.isoformat(),
        lastSeen=last_seen,
    )


def get_device_service(
    access_token: str = Depends(get_bearer_token),
) -> DeviceService:
    return build_device_service(access_token)


@router.get("", response_model=list[DeviceResponse])
async def list_devices(
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    return [_to_response(record) for record in service.list_devices(user_id)]


@router.post("", response_model=DeviceResponse, status_code=201)
async def create_device(
    request: CreateDeviceRequest,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    record = service.create_device(
        user_id,
        name=request.name,
        mac_address=request.macAddress,
    )
    return _to_response(record)


@router.get("/{device_id}", response_model=DeviceResponse)
async def get_device(
    device_id: str,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    return _to_response(service.get_device(user_id, device_id))


@router.patch("/{device_id}", response_model=DeviceResponse)
async def update_device(
    device_id: str,
    request: UpdateDeviceRequest,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    record = service.update_device(
        user_id,
        device_id,
        name=request.name,
    )
    return _to_response(record)


class FeedEventResponse(BaseModel):
    id: str
    created_at: str
    source: str
    repeats: int


@router.get("/{device_id}/feed-events", response_model=list[FeedEventResponse])
async def list_device_feed_events(
    device_id: str,
    limit: int = 20,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    service.get_device(str(current_user.id), device_id)

    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return []

    from supabase import create_client
    client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
    result = (
        client.table("feed_events")
        .select("id, created_at, source, repeats")
        .eq("device_id", device_id)
        .eq("user_id", str(current_user.id))
        .order("created_at", desc=True)
        .limit(min(limit, 100))
        .execute()
    )
    return [
        FeedEventResponse(
            id=row["id"],
            created_at=row["created_at"],
            source=row["source"],
            repeats=int(row["repeats"]),
        )
        for row in result.data
    ]


class DayFeedStat(BaseModel):
    date: str
    count: int
    total_repeats: int


class DeviceStatsResponse(BaseModel):
    feedings_today: int
    total_repeats_today: int
    avg_feedings_per_day: float
    feedings_sparkline: list[int]
    repeats_sparkline: list[int]
    days: list[DayFeedStat]


@router.get("/{device_id}/stats", response_model=DeviceStatsResponse)
async def get_device_stats(
    device_id: str,
    days: int = 7,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    service.get_device(str(current_user.id), device_id)
    days = max(1, min(days, 90))

    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        empty_days = [DayFeedStat(date="", count=0, total_repeats=0)] * days
        return DeviceStatsResponse(
            feedings_today=0,
            total_repeats_today=0,
            avg_feedings_per_day=0.0,
            feedings_sparkline=[0] * days,
            repeats_sparkline=[0] * days,
            days=empty_days,
        )

    from collections import defaultdict
    from datetime import date, datetime, timedelta, timezone

    from supabase import create_client

    client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
    since = (datetime.now(timezone.utc) - timedelta(days=days)).isoformat()
    result = (
        client.table("feed_events")
        .select("created_at, repeats")
        .eq("device_id", device_id)
        .eq("user_id", str(current_user.id))
        .gte("created_at", since)
        .execute()
    )

    counts: dict = defaultdict(int)
    repeats_map: dict = defaultdict(int)
    today_utc = date.today()

    for row in result.data:
        try:
            d = (
                datetime.fromisoformat(row["created_at"].replace("Z", "+00:00"))
                .astimezone(timezone.utc)
                .date()
            )
            counts[d] += 1
            repeats_map[d] += int(row["repeats"])
        except Exception:
            pass

    days_list = [(today_utc - timedelta(days=days - 1 - i)) for i in range(days)]
    feedings_sparkline = [counts.get(d, 0) for d in days_list]
    repeats_sparkline = [repeats_map.get(d, 0) for d in days_list]
    feedings_today = counts.get(today_utc, 0)
    total_repeats_today = repeats_map.get(today_utc, 0)
    avg = sum(feedings_sparkline) / days

    days_detail = [
        DayFeedStat(date=str(d), count=counts.get(d, 0), total_repeats=repeats_map.get(d, 0))
        for d in days_list
    ]

    return DeviceStatsResponse(
        feedings_today=feedings_today,
        total_repeats_today=total_repeats_today,
        avg_feedings_per_day=round(avg, 1),
        feedings_sparkline=feedings_sparkline,
        repeats_sparkline=repeats_sparkline,
        days=days_detail,
    )


class LightEventCreate(BaseModel):
    started_at: str
    ended_at: str
    duration_sec: int


class LightDayStat(BaseModel):
    date: str
    duration_sec: int
    duration_min: int


class LightStatsResponse(BaseModel):
    days: list[LightDayStat]


@router.post("/{device_id}/light-events", status_code=201)
async def create_light_event(
    device_id: str,
    request: LightEventCreate,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    service.get_device(str(current_user.id), device_id)

    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return

    from supabase import create_client

    client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
    client.table("light_events").insert({
        "device_id": device_id,
        "user_id": str(current_user.id),
        "started_at": request.started_at,
        "ended_at": request.ended_at,
        "duration_sec": request.duration_sec,
    }).execute()


@router.get("/{device_id}/light-stats", response_model=LightStatsResponse)
async def get_light_stats(
    device_id: str,
    days: int = 14,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    service.get_device(str(current_user.id), device_id)

    if not SUPABASE_URL or not SUPABASE_SERVICE_KEY:
        return LightStatsResponse(days=[])

    from collections import defaultdict
    from datetime import date, datetime, timedelta, timezone

    from supabase import create_client

    client = create_client(SUPABASE_URL, SUPABASE_SERVICE_KEY)
    since = (datetime.now(timezone.utc) - timedelta(days=days)).isoformat()
    result = (
        client.table("light_events")
        .select("started_at, duration_sec")
        .eq("device_id", device_id)
        .eq("user_id", str(current_user.id))
        .gte("started_at", since)
        .execute()
    )

    duration_map: dict = defaultdict(int)
    today_utc = date.today()

    for row in result.data:
        try:
            d = (
                datetime.fromisoformat(row["started_at"].replace("Z", "+00:00"))
                .astimezone(timezone.utc)
                .date()
            )
            duration_map[d] += int(row["duration_sec"])
        except Exception:
            pass

    days_list = [(today_utc - timedelta(days=days - 1 - i)) for i in range(days)]
    days_detail = [
        LightDayStat(
            date=str(d),
            duration_sec=duration_map.get(d, 0),
            duration_min=round(duration_map.get(d, 0) / 60),
        )
        for d in days_list
    ]

    return LightStatsResponse(days=days_detail)


@router.delete("/{device_id}", status_code=204)
async def delete_device(
    device_id: str,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    service.delete_device(user_id, device_id)
