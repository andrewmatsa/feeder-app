from __future__ import annotations

import pytest

try:
    from backend.device_names import (
        DeviceNameError,
        normalize_device_name,
        suggest_default_device_name,
    )
except ImportError:
    from device_names import DeviceNameError, normalize_device_name, suggest_default_device_name


def test_normalize_device_name_trims_and_collapses_spaces() -> None:
    assert normalize_device_name("  Вітальня  ") == "Вітальня"


def test_normalize_device_name_rejects_too_short() -> None:
    with pytest.raises(DeviceNameError):
        normalize_device_name("  ")


def test_suggest_default_device_name_skips_used_numbers() -> None:
    existing = ["Акваріум 1", "Акваріум 2", "Кухня"]
    assert suggest_default_device_name(existing) == "Акваріум 3"


def test_suggest_default_device_name_starts_at_one() -> None:
    assert suggest_default_device_name([]) == "Акваріум 1"
