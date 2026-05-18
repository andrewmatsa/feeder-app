"""Validation and default display names for user-owned aquarium devices."""

from __future__ import annotations

import re

DEFAULT_NAME_PREFIX = "Акваріум"
MIN_NAME_LENGTH = 2
MAX_NAME_LENGTH = 40


class DeviceNameError(ValueError):
    """Invalid aquarium display name."""


def normalize_device_name(raw: str) -> str:
    name = " ".join(raw.strip().split())
    if len(name) < MIN_NAME_LENGTH:
        raise DeviceNameError(f"Name must be at least {MIN_NAME_LENGTH} characters")
    if len(name) > MAX_NAME_LENGTH:
        raise DeviceNameError(f"Name must be at most {MAX_NAME_LENGTH} characters")
    return name


def suggest_default_device_name(existing_names: list[str]) -> str:
    """Pick the next free 'Акваріум N' label for this account."""
    used_numbers: set[int] = set()
    pattern = re.compile(rf"^{re.escape(DEFAULT_NAME_PREFIX)}\s+(\d+)$")

    for name in existing_names:
        match = pattern.match(name.strip())
        if match:
            used_numbers.add(int(match.group(1)))

    number = 1
    while number in used_numbers:
        number += 1
    return f"{DEFAULT_NAME_PREFIX} {number}"
