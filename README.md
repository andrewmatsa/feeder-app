# 🐟 AquaFeed - Automatic Fish Feeder

AquaFeed is an ESP32-C3 based automatic fish feeder with firmware control, a Python backend adapter, and a React frontend.

## Stack

- Firmware: ESP32-C3, C++, Arduino framework, PlatformIO
- Backend: Python, FastAPI, httpx, Pydantic
- Frontend: React, TypeScript, Vite, Axios

## Features

- Scheduled automatic feeding
- Manual feeding from hardware button or API
- Servo speed and angle control
- Battery monitoring
- Power-saving and deep-sleep controls
- OTA firmware updates via web interface (no USB cable required)
- Embedded device web UI plus a separate SPA frontend

## Project Structure

```text
feeder-app/
├── src/                    # ESP32 firmware
├── backend/                # FastAPI adapter for frontend -> firmware
├── frontend/               # React SPA
├── versions.json           # Single source of truth for app/firmware version
├── platformio.ini          # PlatformIO configuration
└── README.md
```

## Architecture

The project currently has three active layers:

1. `src/` firmware exposes the device API and controls hardware.
2. `backend/` exposes a stable API contract and maps requests to the firmware routes.
3. `frontend/` consumes the backend API instead of talking directly to the ESP32.

Recommended request flow:

`Frontend -> FastAPI backend -> ESP32 firmware API`

## Versioning

Runtime version values are centralized in `versions.json`.

- `appVersion` is used by the backend API metadata and the React frontend.
- `firmwareVersion` is injected into the ESP32 firmware build and shown in device-facing UI.

If you need to bump a release, update `versions.json` first and treat it as the authoritative source.

## Firmware Setup

1. Install [PlatformIO](https://platformio.org/).
2. Open the project.
3. Build and upload the firmware to the ESP32-C3:

```bash
pio run --target upload
```

4. On first boot the device creates an access point for WiFi provisioning.

After the device joins WiFi, note its hostname or IP from the serial monitor and use that value for the backend `ESP32_BASE_URL`.

### OTA Partition Table

The firmware uses `min_spiffs.csv` to enable OTA updates (two app partitions ~1.9 MB each). **The first flash must be done over USB** to write the new partition table. After that, all firmware updates can be done wirelessly through the Settings → Firmware Update section in the web app.

To build a firmware binary for OTA upload:

```bash
pio run
# Binary is at .pio/build/esp32-c3-devkitc-02/firmware.bin
```

## Backend Setup

From the `backend/` directory:

```bash
pip install -r requirements.txt
```

Create a `.env` file from `env.example` and set:

```env
ESP32_BASE_URL=http://<device-host-or-ip>
CORS_ORIGINS=http://localhost:5173,http://localhost:3000
PORT=8000
```

Run the API:

```bash
uvicorn main:app --reload --port 8000
```

## Frontend Setup

From the `frontend/` directory:

```bash
npm install
npm run dev
```

With Docker (recommended below), API calls go through the Vite proxy on port `5173` — do not set `VITE_API_URL` in the frontend container.

Without Docker, either use the proxy (`npm run dev`, no `VITE_API_URL`) or set `VITE_API_URL=http://localhost:8000` in `frontend/.env.local`.

## Docker (dev)

The stack runs backend + frontend in containers. The browser talks only to `http://localhost:5173`; Vite proxies `/api` and `/auth` to the backend service.

1. Copy env for the backend:

```bash
cp backend/env.docker.example backend/.env
```

Edit `backend/.env`:

- **Supabase** — required for login and **Мої акваріуми** (`/api/v1/devices`).
- **`ESP32_BASE_URL`** — LAN IP of the feeder (e.g. `http://192.168.1.50`). `fish.local` usually does not resolve inside Docker; use the device IP from the serial monitor.

2. Start:

```bash
docker compose up --build
```

3. Open:

- SPA: [http://localhost:5173](http://localhost:5173) — register/login → **Мої акваріуми**
- API docs: [http://localhost:8000/docs](http://localhost:8000/docs)

```text
Browser :5173  --/api, /auth-->  frontend (Vite)  --proxy-->  backend :8000  --ESP32_BASE_URL-->  feeder (LAN)
```

## Test Pages

The repository also includes dedicated offline preview pages so UI changes can be checked without flashing the ESP32 every time.

- `preview.html` is the hub for the preview/test harness.
- `preview_index.html` covers the main control screen.
- `preview_info.html` covers the diagnostics and system info screen.
- `preview_wifi.html` covers WiFi and power/display settings.

Use these pages for layout, copy, and interaction checks with mocked data before moving on to backend integration or hardware validation.

## Canonical Backend API

The backend is the public API surface for clients. It normalizes the current firmware payloads and routes.

### `GET /api/status`

```json
{
  "angle": 90,
  "speed": 20,
  "feedRepeats": 1,
  "powerSaveMode": true,
  "displayEnabled": true,
  "displayOffAfterSec": 20,
  "deepSleepIdleSec": 60,
  "batteryVoltage": 3.7,
  "batteryPercent": 75,
  "feedTimes": [
    { "hour": 8, "minute": 0, "repeats": 1 },
    { "hour": 20, "minute": 0, "repeats": 2 }
  ],
  "nextFeedMinutes": 120,
  "nextFeedHour": 20,
  "nextFeedMinute": 0,
  "currentTime": "18:00",
  "manualFeedCooldownSeconds": 0,
  "wifiSSID": "AquariumWiFi",
  "wifiIP": "192.168.1.50",
  "isAPMode": false,
  "sleepReason": "ready",
  "sleepCountdownSeconds": 15,
  "displayAwake": true,
  "timestamp": "2026-04-22T12:00:00"
}
```

### `POST /api/feed`

```json
{ "repeats": 1 }
```

### `POST /api/speed`

```json
{ "speed": 20 }
```

### `POST /api/angle`

```json
{ "angle": 90 }
```

### `POST /api/power-mode`

```json
{ "enabled": true }
```

### `POST /api/schedule`

```json
{
  "times": [
    { "hour": 8, "minute": 0, "repeats": 1 },
    { "hour": 20, "minute": 0, "repeats": 2 }
  ]
}
```

### `POST /api/ota-update`

Upload a firmware `.bin` file as `multipart/form-data`. The backend streams it directly to the ESP32 without buffering the full binary in RAM.

```
Content-Type: multipart/form-data
Body: file=<firmware.bin>
```

Response on success:

```json
{ "success": true, "message": "Firmware update complete, device rebooting" }
```

The device reboots automatically after flashing. The frontend polls `/api/status` until the device responds again (up to 90 seconds). Returns `400` if the file is not a `.bin`, `412` if battery is below 20%, `504` on timeout, `502` if the flash itself failed.

## Notes

- The firmware still has its own embedded UI and internal route naming.
- The backend adapter exists to shield the frontend from those firmware-specific details.
- If you expand the project further, prefer adding new client features against the backend contract, not directly against the firmware API.

