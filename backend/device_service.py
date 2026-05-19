"""Device registry: list, create, rename, delete — keyed by user-facing aquarium name."""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Protocol
from uuid import uuid4

from fastapi import HTTPException, status

try:
    from .device_names import DeviceNameError, normalize_device_name, suggest_default_device_name
except ImportError:
    from device_names import DeviceNameError, normalize_device_name, suggest_default_device_name


@dataclass
class DeviceRecord:
    id: str
    user_id: str
    name: str
    mac_address: str | None
    created_at: datetime
    last_seen: datetime | None


def resolve_device_name(name: str | None, existing_names: list[str]) -> str:
    if name is None or not name.strip():
        return suggest_default_device_name(existing_names)
    try:
        return normalize_device_name(name)
    except DeviceNameError as exc:
        raise HTTPException(status_code=status.HTTP_422_UNPROCESSABLE_ENTITY, detail=str(exc)) from exc


def ensure_unique_name(
    devices: list[DeviceRecord],
    user_id: str,
    name: str,
    *,
    exclude_id: str | None = None,
) -> None:
    for device in devices:
        if device.user_id != user_id:
            continue
        if exclude_id and device.id == exclude_id:
            continue
        if device.name.casefold() == name.casefold():
            raise HTTPException(
                status_code=status.HTTP_409_CONFLICT,
                detail=f"An aquarium named '{name}' already exists on this account",
            )


def ensure_unique_mac(
    devices: list[DeviceRecord],
    mac_address: str,
    *,
    exclude_id: str | None = None,
) -> None:
    normalized = mac_address.strip().lower()
    for device in devices:
        if exclude_id and device.id == exclude_id:
            continue
        if device.mac_address and device.mac_address.lower() == normalized:
            raise HTTPException(
                status_code=status.HTTP_409_CONFLICT,
                detail="This device is already registered",
            )


class DeviceService(Protocol):
    def list_devices(self, user_id: str) -> list[DeviceRecord]: ...

    def get_device(self, user_id: str, device_id: str) -> DeviceRecord: ...

    def create_device(
        self,
        user_id: str,
        *,
        name: str | None = None,
        mac_address: str | None = None,
    ) -> DeviceRecord: ...

    def update_device(
        self,
        user_id: str,
        device_id: str,
        *,
        name: str | None = None,
    ) -> DeviceRecord: ...

    def delete_device(self, user_id: str, device_id: str) -> None: ...


class InMemoryDeviceService:
    """Test/dev store when Supabase is not configured."""

    def __init__(self) -> None:
        self._devices: dict[str, DeviceRecord] = {}

    def list_devices(self, user_id: str) -> list[DeviceRecord]:
        items = [d for d in self._devices.values() if d.user_id == user_id]
        return sorted(items, key=lambda d: d.created_at)

    def get_device(self, user_id: str, device_id: str) -> DeviceRecord:
        record = self._devices.get(device_id)
        if record is None or record.user_id != user_id:
            raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
        return record

    def create_device(
        self,
        user_id: str,
        *,
        name: str | None = None,
        mac_address: str | None = None,
    ) -> DeviceRecord:
        existing = self.list_devices(user_id)
        display_name = resolve_device_name(name, [d.name for d in existing])
        ensure_unique_name(list(self._devices.values()), user_id, display_name)
        if mac_address:
            ensure_unique_mac(list(self._devices.values()), mac_address)

        now = datetime.now(timezone.utc)
        record = DeviceRecord(
            id=str(uuid4()),
            user_id=user_id,
            name=display_name,
            mac_address=mac_address,
            created_at=now,
            last_seen=None,
        )
        self._devices[record.id] = record
        return record

    def update_device(
        self,
        user_id: str,
        device_id: str,
        *,
        name: str | None = None,
    ) -> DeviceRecord:
        record = self.get_device(user_id, device_id)
        if name is not None:
            display_name = resolve_device_name(name, [d.name for d in self.list_devices(user_id)])
            ensure_unique_name(list(self._devices.values()), user_id, display_name, exclude_id=device_id)
            record = DeviceRecord(
                id=record.id,
                user_id=record.user_id,
                name=display_name,
                mac_address=record.mac_address,
                created_at=record.created_at,
                last_seen=record.last_seen,
            )
        self._devices[record.id] = record
        return record

    def delete_device(self, user_id: str, device_id: str) -> None:
        record = self.get_device(user_id, device_id)
        del self._devices[record.id]


class SupabaseDeviceService:
    """Persist devices in Supabase with RLS via the user's access token."""

    def __init__(self, access_token: str) -> None:
        self._access_token = access_token

    def _client(self):
        try:
            from .supabase_client import get_supabase
        except ImportError:
            from supabase_client import get_supabase

        client = get_supabase()
        client.postgrest.auth(self._access_token)
        return client

    def list_devices(self, user_id: str) -> list[DeviceRecord]:
        response = (
            self._client()
            .table("devices")
            .select("*")
            .eq("user_id", user_id)
            .order("created_at")
            .execute()
        )
        return [_row_to_record(row) for row in response.data or []]

    def get_device(self, user_id: str, device_id: str) -> DeviceRecord:
        response = (
            self._client()
            .table("devices")
            .select("*")
            .eq("id", device_id)
            .eq("user_id", user_id)
            .limit(1)
            .execute()
        )
        rows = response.data or []
        if not rows:
            raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
        return _row_to_record(rows[0])

    def create_device(
        self,
        user_id: str,
        *,
        name: str | None = None,
        mac_address: str | None = None,
    ) -> DeviceRecord:
        existing = self.list_devices(user_id)
        display_name = resolve_device_name(name, [d.name for d in existing])
        ensure_unique_name(existing, user_id, display_name)
        if mac_address:
            ensure_unique_mac(existing, mac_address)

        payload = {
            "user_id": user_id,
            "name": display_name,
            "mac_address": mac_address,
        }
        try:
            response = self._client().table("devices").insert(payload).execute()
        except Exception as exc:
            _raise_supabase_conflict(exc)
            raise
        rows = response.data or []
        if not rows:
            raise HTTPException(status_code=status.HTTP_500_INTERNAL_SERVER_ERROR, detail="Failed to create device")
        return _row_to_record(rows[0])

    def update_device(
        self,
        user_id: str,
        device_id: str,
        *,
        name: str | None = None,
    ) -> DeviceRecord:
        existing = self.list_devices(user_id)
        if name is not None:
            display_name = resolve_device_name(name, [d.name for d in existing])
            ensure_unique_name(existing, user_id, display_name, exclude_id=device_id)
            name = display_name

        payload: dict = {}
        if name is not None:
            payload["name"] = name
        if not payload:
            return self.get_device(user_id, device_id)

        try:
            response = (
                self._client()
                .table("devices")
                .update(payload)
                .eq("id", device_id)
                .eq("user_id", user_id)
                .execute()
            )
        except Exception as exc:
            _raise_supabase_conflict(exc)
            raise
        rows = response.data or []
        if not rows:
            return self.get_device(user_id, device_id)
        return _row_to_record(rows[0])

    def delete_device(self, user_id: str, device_id: str) -> None:
        self.get_device(user_id, device_id)
        self._client().table("devices").delete().eq("id", device_id).eq("user_id", user_id).execute()


def _row_to_record(row: dict) -> DeviceRecord:
    return DeviceRecord(
        id=str(row["id"]),
        user_id=str(row["user_id"]),
        name=row["name"],
        mac_address=row.get("mac_address"),
        created_at=_parse_datetime(row["created_at"]),
        last_seen=_parse_datetime(row["last_seen"]) if row.get("last_seen") else None,
    )


def _parse_datetime(value: str | datetime) -> datetime:
    if isinstance(value, datetime):
        return value
    text = str(value).replace("Z", "+00:00")
    return datetime.fromisoformat(text)


def _raise_supabase_conflict(exc: Exception) -> None:
    message = str(exc).lower()
    if "duplicate" in message or "unique" in message or "23505" in message:
        if "name" in message or "devices_user_name_unique" in message:
            raise HTTPException(
                status_code=status.HTTP_409_CONFLICT,
                detail="An aquarium with this name already exists on this account",
            ) from exc
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="This device is already registered",
        ) from exc


def build_device_service(access_token: str) -> DeviceService:
    try:
        from .config import SUPABASE_ANON_KEY, SUPABASE_URL
    except ImportError:
        from config import SUPABASE_ANON_KEY, SUPABASE_URL

    if SUPABASE_URL and SUPABASE_ANON_KEY:
        return SupabaseDeviceService(access_token)
    return InMemoryDeviceService()
