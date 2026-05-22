# CLAUDE.md

This file provides repository-specific guidance for AI coding agents working in this project.

## Project Overview

AquaFeed is a battery-powered automatic fish feeder with three active layers:

1. **Firmware** — ESP32-C3, C++/Arduino, PlatformIO
2. **Backend** — FastAPI adapter that normalizes firmware API calls
3. **Frontend** — React SPA (TypeScript/Vite) that communicates only through the backend

Primary product themes:

- battery-aware behavior
- deep sleep and wake-up control
- Wi-Fi provisioning and local device control
- stable backend contract for clients
- responsive web UI for both device pages and SPA

## Non-Negotiables

- The frontend must never call firmware routes directly. The supported flow is:

```text
Frontend -> Backend -> ESP32 firmware
```

- The backend is the public contract layer. If firmware payloads change, update backend mappers and frontend types together.
- `versions.json` is the single source of truth for version strings across firmware, backend, and frontend.
- Battery-first behavior is the default product assumption. Do not trade away deep sleep or low-power behavior casually.
- Treat wake/sleep, Wi-Fi power behavior, and hardware assumptions as product behavior, not incidental implementation details.

## Architecture Guardrails

### Firmware

- Main entry and pin assignments live in `src/main.cpp`.
- Runtime orchestration lives in `src/device_runtime.*`.
- HTTP handlers live in `src/api_handlers.*`.
- Embedded HTML/CSS/JS pages live in `src/web/`.
- Settings persist through Arduino `Preferences` (NVS).

Hardcoded pins in `src/main.cpp`:

| Pin | Function |
|-----|----------|
| GPIO 4 | Servo (PWM) |
| GPIO 3 | Feed button (wakeup source) |
| GPIO 2 | Battery ADC |
| GPIO 5 | LDR light sensor (voltage divider: 3.3V → LDR → GPIO5 → 10kΩ → GND) |
| GPIO 6 | OLED SDA |
| GPIO 7 | OLED SCL |

**GPIO 3 button behavior (`src/main.cpp` → `handleManualButtonPress()`):**

| Press duration | Action |
|----------------|--------|
| Short press (<3s) | Manual feed sequence |
| Long press (≥5s) | Enter AP mode (`FishFeeder-<MAC>`) for WiFi reconfiguration |

To change WiFi when the device is unreachable: hold the button for 3+ seconds → connect to `FishFeeder-XXXX` (password: `12345678`) → open `192.168.4.1/wifi`.

### Backend

- `backend/main.py` exposes the stable API surface.
- `backend/device_client.py` forwards requests to the device firmware.
- `backend/mappers.py` is the normalization boundary between firmware payloads and public backend models.
- `backend/models.py` defines the backend request/response contracts.

### Frontend

- The SPA talks only to the backend.
- Keep device-specific protocol details out of the frontend whenever possible.
- `VITE_API_URL` controls the backend base URL.

## Power and Hardware Rules

- Battery life is a first-class requirement.
- Prefer deep sleep, short online windows, and explicit power-state UX over always-online behavior.
- Any change to sleep, wake, Wi-Fi reconnect, or polling behavior must consider battery impact.
- Any change to servo, OLED, battery sensing, or wake sources must consider hardware implications.
- Do not change wake semantics, sleep timers, or GPIO assumptions without verifying the intended hardware behavior.
- If low-power optimization depends on hardware power gating, say that explicitly instead of pretending firmware alone solves it.

## Embedded UI Rules

- The device web UI is not secondary; it is part of the product.
- When changing layout or interaction in `src/web/*.cpp`, also verify the corresponding preview pages:
  - `preview_index.html`
  - `preview_info.html`
  - `preview_wifi.html`
- UI must communicate battery, sleep, offline, AP mode, and next-feed state clearly.
- Keep bilingual UX in mind when changing labels or layout.
- Mobile-first is fine, but do not make desktop an afterthought.

## Change Rules

- If firmware status fields change, update all affected layers:
  - firmware JSON output
  - backend mappers/models
  - frontend types/UI consumers
- If adding a new setting, wire it through all required layers intentionally rather than leaving partial support.
- If changing power-management behavior, document what changes for:
  - manual wake
  - scheduled wake
  - web activity
  - battery usage
- If changing Wi-Fi or AP flows, preserve provisioning clarity for non-technical users.

## Validation Expectations

Run the minimum relevant validation for the layer you touched.

### Firmware (ESP32-C3)

```bash
pio run
pio run --target upload
pio run --target monitor
pio run --target upload --target monitor
```

To build a firmware binary for OTA upload (output: `.pio/build/esp32-c3-devkitc-02/firmware.bin`):

```bash
pio run
```

**OTA note:** The firmware uses `min_spiffs.csv` partition table (two OTA app partitions). The first flash after adding this partition scheme must be done over USB to write the new partition layout. After that, firmware can be updated wirelessly via Settings → Firmware Update in the web app.

### Backend (Python/FastAPI)

```bash
cd backend
pip install -r requirements.txt
cp env.example .env
uvicorn main:app --reload --port 8000
```

### Backend Tests

Mock tests (no hardware required):
```bash
cd backend
pytest -m mock -v
```

Hardware tests (real ESP32 must be reachable):
```bash
cd backend
pytest -m hardware --device-url http://192.168.1.100 -v
```

Test markers:
- `mock` — uses `MOCK_DEVICE=True` or patches `request_firmware`; safe for CI
- `hardware` — calls real firmware; requires `--device-url`

### Frontend (TypeScript/React)

```bash
cd frontend
npm install
npm run dev
npm run build
npm run lint
npm run preview
```

Validation priorities:

- Firmware changes: build at minimum; mention any hardware verification that was not performed.
- Backend changes: verify route behavior and model shape assumptions.
- Frontend changes: `npm run lint` and `npm run build`.
- Embedded UI changes: check preview pages when possible.

## Git and Repository Hygiene

- Do not commit local AI tooling state such as `.claude/`.
- Commit `.agents/skills/` only when they are intentional project tooling, not personal scratch files.
- Avoid unrelated file churn while making focused changes.
- Do not commit generated local-only files unless the repository intentionally treats them as source.

## Practical Notes

- Preview pages exist for offline UI work without flashing hardware every time.
- After Wi-Fi provisioning, the device is reachable via its local IP (mDNS `fish.local` may work on some networks but is not reliable in production — use the backend as the stable entry point).
- There are currently no automated tests; ESLint is the only enforced QA tool in the frontend.
- When in doubt, prefer small changes with explicit behavior over large speculative refactors.
