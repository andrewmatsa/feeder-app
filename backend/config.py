"""Backend configuration and environment loading."""

import json
import os
from pathlib import Path

from dotenv import load_dotenv

load_dotenv()

VERSIONS_FILE = Path(__file__).resolve().parent.parent / "versions.json"
VERSIONS = json.loads(VERSIONS_FILE.read_text(encoding="utf-8"))

CORS_ORIGINS = os.getenv(
    "CORS_ORIGINS",
    "http://localhost:5173,http://localhost:3000",
).split(",")
ESP32_BASE_URL = os.getenv("ESP32_BASE_URL", "http://fish-eat.local")
APP_VERSION = VERSIONS["appVersion"]
FIRMWARE_VERSION = VERSIONS["firmwareVersion"]
