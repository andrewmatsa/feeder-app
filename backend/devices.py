"""User-owned aquarium devices (display names for multiple feeders per account)."""

from fastapi import APIRouter, Depends
from pydantic import BaseModel, Field

try:
    from .dependencies import UserClaims, get_bearer_token, get_current_user
    from .device_service import DeviceRecord, DeviceService, build_device_service
except ImportError:
    from dependencies import UserClaims, get_bearer_token, get_current_user
    from device_service import DeviceRecord, DeviceService, build_device_service

router = APIRouter(prefix="/api/v1/devices", tags=["devices"])


class DeviceResponse(BaseModel):
    id: str
    name: str
    macAddress: str | None = None
    sortOrder: int = 0
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
    sortOrder: int | None = Field(default=None, ge=0)


def _to_response(record: DeviceRecord) -> DeviceResponse:
    return DeviceResponse(
        id=record.id,
        name=record.name,
        macAddress=record.mac_address,
        sortOrder=record.sort_order,
        createdAt=record.created_at.isoformat(),
        lastSeen=record.last_seen.isoformat() if record.last_seen else None,
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
        sort_order=request.sortOrder,
    )
    return _to_response(record)


@router.delete("/{device_id}", status_code=204)
async def delete_device(
    device_id: str,
    current_user: UserClaims = Depends(get_current_user),
    service: DeviceService = Depends(get_device_service),
):
    user_id = str(current_user.id)
    service.delete_device(user_id, device_id)
