from __future__ import annotations

import pytest
from fastapi import HTTPException

try:
    from backend.device_service import InMemoryDeviceService
except ImportError:
    from device_service import InMemoryDeviceService


@pytest.fixture
def service() -> InMemoryDeviceService:
    return InMemoryDeviceService()


USER_ID = "user-test-0001"


def test_create_device_with_display_name(service: InMemoryDeviceService) -> None:
    record = service.create_device(USER_ID, name="Вітальня")
    assert record.name == "Вітальня"


def test_create_device_assigns_default_aquarium_names(service: InMemoryDeviceService) -> None:
    first = service.create_device(USER_ID)
    second = service.create_device(USER_ID)
    assert first.name == "Акваріум 1"
    assert second.name == "Акваріум 2"


def test_duplicate_name_returns_conflict(service: InMemoryDeviceService) -> None:
    service.create_device(USER_ID, name="Спальня")
    with pytest.raises(HTTPException) as exc:
        service.create_device(USER_ID, name="спальня")
    assert exc.value.status_code == 409


def test_rename_device(service: InMemoryDeviceService) -> None:
    created = service.create_device(USER_ID, name="Офіс")
    updated = service.update_device(USER_ID, created.id, name="Дитяча")
    assert updated.name == "Дитяча"


def test_list_devices_returns_all_aquariums(service: InMemoryDeviceService) -> None:
    service.create_device(USER_ID, name="Вітальня")
    service.create_device(USER_ID, name="Спальня")
    names = {device.name for device in service.list_devices(USER_ID)}
    assert names == {"Вітальня", "Спальня"}
