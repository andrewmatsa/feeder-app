# React Native (Expo) mobile app for AquaFeed — phased plan

## Context

The product currently has three layers (Frontend SPA -> Backend -> Firmware) per `CLAUDE.md`'s non-negotiable contract. The owner wants a real mobile app (Android + iOS) with native-level performance rather than a wrapped webview, but is worried about time investment.

Decision from discussion: use **Expo** (fast iteration, EAS Build/OTA, easy push later) instead of bare RN CLI. MVP scope is **Home + Info + Feed + Devices list + Auth** — Settings and Stats screens are phase 2, Admin panel is not ported (it's an owner-only tool, not an end-user surface). **Push notifications are deferred to a later phase**, added once the MVP is stable, since they require backend work (device push token storage, event triggers on low battery/offline/missed feed).

Why this is low-risk time-wise: the backend (`backend/main.py`, `backend/devices.py`) is already a stable, generic REST API with Bearer-token auth (`backend/dependencies.py` via Supabase, but auth is done through custom `/auth/login`, `/auth/refresh`, `/auth/logout` endpoints — not the Supabase JS SDK directly). This means the RN app talks to the exact same backend contract the SPA already uses (`frontend/src/services/api.ts`), so **zero backend changes are needed for the MVP** — only a new client consuming the same endpoints. The auth token-refresh interceptor pattern in `frontend/src/services/api.ts:63-116` can be ported almost 1:1 to an axios instance in RN (swap `localStorage` for `expo-secure-store` / `AsyncStorage`).

## Non-negotiables carried over from CLAUDE.md

- Mobile app must talk **only to the backend**, never directly to firmware (same as SPA rule).
- `versions.json` stays the single source of truth for version strings — mobile app version tracked separately but firmware/backend contract stays unchanged.
- No firmware or backend contract changes required for MVP phases (1–4). Push notifications phase (5) is the only one touching the backend.

## Repo layout

New top-level `mobile/` directory (sibling to `frontend/`, `backend/`), Expo (TypeScript) project, independent `package.json`. Do not touch `frontend/` — it keeps serving as the web SPA and is not going away.

## Phase 1 — Project scaffold & auth (foundation)

- `npx create-expo-app mobile --template` (TypeScript, Expo Router for file-based navigation — maps cleanly to the existing route structure in `frontend/src/App.tsx`).
- Port `authStorage` + `apiClient` + refresh-token interceptor logic from `frontend/src/services/api.ts` into `mobile/src/services/api.ts`, replacing `localStorage` with `expo-secure-store` (tokens are sensitive — SecureStore, not AsyncStorage).
- Port `types.ts` (shared shape currently only in frontend — copy, don't share a package yet, to avoid monorepo tooling overhead for MVP).
- Screens: Login, Register (mirror `frontend/src/pages/LoginPage.tsx`, `RegisterPage.tsx` logic, native `TextInput`/`Pressable` instead of DOM forms).
- Auth state: port `frontend/src/store/authStore.ts` (zustand) as-is — zustand works unchanged in RN.
- Verification: log in against the running backend (`uvicorn main:app --reload`) from an Expo Go dev client on a physical/simulated device; confirm token refresh on 401.

## Phase 2 — Devices list & add device

- Port `frontend/src/pages/DevicesPage.tsx` and `AddDevicePage.tsx` logic to native list/form screens using `api.listDevices`, `api.createDevice`.
- Navigation: Expo Router stack — `/devices` (list) -> `/devices/[deviceId]` (dashboard, phase 3).
- Verification: create/list/delete a device end-to-end against the mock backend (`MOCK_DEVICE=True`).

## Phase 3 — Device dashboard: Home + Info tabs + Feed action

- This is the core value screen. Port `frontend/src/pages/DeviceDashboardPage.tsx`'s Home and Info tabs only (Settings/Stats tabs excluded from MVP per scope decision).
- Battery gauge: the SVG arc gauge (`DeviceDashboardPage.tsx:20-80`) needs a native equivalent — use `react-native-svg` (Expo-compatible) to port the same `describeArc` math directly; this is a straight port, not a redesign.
- Feed button -> `api.feedNow`. Status polling -> `api.getStatus`.
- Bilingual UX: port `frontend/src/translations.ts` (`TRANSLATIONS`, `Lang`) unchanged; language toggle stored via SecureStore/AsyncStorage instead of `localStorage`.
- Verification: run against mock backend, confirm feed action, battery gauge rendering, and offline/AP-mode state messaging all match SPA behavior described in CLAUDE.md's Embedded UI Rules (battery, sleep, offline, next-feed state must be clear).

## Phase 4 — Polish & release prep

- App icons, splash screen, `app.json` config (bundle ID for iOS, package name for Android).
- Error/loading states, pull-to-refresh on status.
- EAS Build setup for both platforms; TestFlight (iOS) and internal testing track (Android) for owner/beta testing before public release.
- Verification: full manual pass on both a physical Android device and iOS device/simulator against the real backend + real firmware (not mock), covering login, device list, feed, battery/offline states.

## Phase 5 — Push notifications (implemented)

**Conflict check against web:** confirmed zero existing push/notification code anywhere in `frontend/` or `backend/` prior to this (grepped both — no web push subscriptions, no notification tables, no scheduler). This is a net-new, mobile-only feature; the web SPA is untouched and unaffected.

**Backend:**
- `backend/supabase_migrations/005_push_tokens.sql` — new `push_tokens` table (user_id, device_id, token, platform), RLS scoped to owner. **Not yet applied** — run it in the Supabase Dashboard SQL editor like migrations 001–004 before push will work end-to-end.
- `backend/push_notifications.py` — `register_push_token()`, `send_expo_push()` (Expo push API), and `run_alert_check_loop()`: a background `asyncio` task (started in `main.py`'s `lifespan`) that polls every 5 min for devices with at least one registered token, checks **low battery** and **offline** conditions, and sends a push on state transition (6h re-notify cooldown, dedupe state stored in the existing `devices.settings` JSONB column — no new columns needed).
- `backend/devices.py` — new authenticated `POST /api/v1/devices/{device_id}/push-token` endpoint.
- **Deliberately deferred:** "missed scheduled feed" alerts — needs comparing the schedule against actual `feed_events` over time, a heavier feature than battery/offline. Only battery-low + offline shipped in this pass.

**Mobile:**
- `mobile/src/services/pushNotifications.ts` — `registerForPushNotificationsAsync()`: requests permission, gets Expo push token. Fails safe (returns `null`) whenever push isn't available.
- `mobile/app/devices/index.tsx` — registers the token against every device the user owns once the list loads.

**Known blockers to actually testing this, not code gaps:**
1. ~~Migration 005 needs to be run manually in Supabase.~~ **Done** — `push_tokens` table applied and confirmed queryable.
2. **`expo-notifications` remote push does not work in Expo Go** (dropped since SDK 53) — this whole session's testing has been via Expo Go, so push tokens will silently come back `null` there. Testing requires a real EAS dev-client/build (`eas login` + `eas build --profile development`).
3. **Deferred by owner decision** — no EAS project exists yet (`extra.eas.projectId` unset in `app.config.js`). Run `eas init` to generate one, then add it to `app.config.js`, when ready to actually test push end-to-end. Until then, `registerForPushNotificationsAsync()` keeps returning `null` and the feature stays inert (no crashes, no partial state) — safe to leave as-is in the meantime.

## Phase 7 — Settings tab

Port `frontend/src/pages/DeviceDashboardPage.tsx`'s `renderSettings()` (~line 1347) to a new `Settings` tab in `mobile/app/devices/[deviceId].tsx`, reusing the existing `api.*` mutator calls already ported in `mobile/src/services/api.ts` (Phase 1) — no new backend or API-client work needed, this is purely a new screen consuming endpoints that already exist.

Sections to port, each its own card exactly like the web version:
- **Language** — mobile already has a lang toggle in the dashboard header (`[deviceId].tsx`); either keep it there or move it into this tab for parity, and persist the choice (currently resets on app restart — worth fixing here since Settings is where users expect it).
- **Power settings** — power-save toggle + deep sleep idle seconds (`api.setDisplaySettings`).
- **OLED display** — enabled toggle + display-off-after seconds (`api.setDisplaySettings`).
- **Feed interval** — minimum feed interval minutes (`api.setMinInterval`).
- **Battery calibration** — actual voltage input (`api.calibrateBattery`).
- **Timezone** — UTC offset picker (`api.setTimezone`); RN has no native `<select>` — use a simple modal/action-sheet list or a horizontal stepper instead.
- **OTA firmware update** — file picker (`.bin`) + upload + reboot-polling (`api.otaUpdate` — note the web version streams a `File`; RN needs `expo-document-picker` to get a file URI, then upload via `FormData` with `{ uri, name, type }` per React Native's fetch/FormData convention, not a browser `File` object). Reuse the same step-indicator UX (select → uploading → done) and the low-battery warning gate (`status.batteryPercent < 20`) from the web version.
- **Danger zone — Forget WiFi** — `api.forgetWifi`, with a native `Alert.alert` confirmation (same pattern already used for device deletion in `mobile/app/devices/index.tsx`).

Explicitly **not** porting into this phase (stays web-only / out of scope, see below):
- **Food supply tracking** — pure `localStorage` feature with no backend, not core to device control.
- **Light sensor toggle/threshold** — per `CLAUDE.md`, GPIO5 (LDR) is hardware-disabled in firmware (ADC2 unsupported on ESP32-C3), so this setting has no real effect on this hardware.

Verification: exercise every setting against `MOCK_DEVICE=True` first (mock responses), then confirm each mutator against the real backend/firmware, matching the plan's existing verification pattern from Phases 1–3. OTA specifically needs a real firmware `.bin` and a real device — do not skip this one on mock alone given it reboots physical hardware.

## Phase 6 — BLE device provisioning (deferred, backlog idea)

Not part of MVP execution scope — captured here so it isn't lost. Replace the AP-mode WiFi pairing flow (connect to `FishFeeder-XXXX`, open `192.168.4.1/wifi`) with BLE provisioning: the app sends home WiFi credentials directly to the device over Bluetooth, without the user leaving the app or manually switching WiFi networks.

- **Why:** AP-mode requires the user to exit the app, join a networkless WiFi hotspot (iOS shows scary "No Internet" warnings for this), configure credentials in a browser, then switch back. BLE keeps the whole flow inside the app — objectively less confusing for non-technical users.
- **Cost/trade-off:** this is a real scope increase, not a tweak:
  - Firmware: new BLE GATT service on the ESP32-C3 (new C++ code, new hardware testing, coexistence with WiFi radio use needs verification per CLAUDE.md's hardware-change rules).
  - Mobile: requires a native BLE library (e.g. `react-native-ble-plx`), which **does not work in Expo Go** — needs a custom EAS dev client. Also needs BLE permissions (Android location permission for BLE scan pre-Android 12 is a UX wrinkle of its own).
- **Decision:** revisit only after the MVP (Phases 1–4) ships and real users show the AP-mode pairing flow is actually a problem, not a hypothetical one. Do not build speculatively.

## Phase 8 — Full web parity: Servo control, Light sensor, Food supply, Stats tab (implemented)

After Phase 7, a full section-by-section audit against `frontend/src/pages/DeviceDashboardPage.tsx` found remaining gaps and closed all of them:

- **Servo control** (Home tab): angle slider (debounced live send via `api.setAngle`) + speed slider (explicit save via `api.setSpeed`) — needed `@react-native-community/slider`, RN has no built-in slider.
- **Light sensor**: gauge on Home (matches `LightGauge` from web, session-duration tracking ported to `mobile/app/devices/[deviceId].tsx`), toggle+threshold in Settings, and light-event persistence via `api.createLightEvent` — same as web, even though GPIO5/LDR is hardware-disabled on this ESP32-C3 board (see CLAUDE.md) so this is UI/data-shape parity, not a functional sensor reading.
- **Food supply**: percentage bar + remaining/duration estimate on Home, gram inputs in Settings. Client-side-only estimate, no backend — ported the same `localStorage`-equivalent (SecureStore) keys as web, including web's own quirk of the total/loaded-at/grams-per-feed being global rather than per-device (kept for parity, not a new bug).
- **Stats tab** (new 4th tab): summary cards, 7/14/30-day period selector, feedings + light sparklines (`Sparkline` ported to `react-native-svg`), and the same conditional + daily-tip recommendations logic as web. Uses `api.getDeviceStats`/`api.getLightStats` (both already existed on the backend, only the mobile client needed the methods).
- All new section icons (servo, sun/light, food list, stats bar-chart tab icon) ported from web's exact SVG paths into `mobile/src/components/SectionIcon.tsx` / `TabIcons.tsx`.

Mobile now has full feature parity with the web SPA's device dashboard (Home/Info/Settings/Stats). Only the **Admin panel** remains web-only, by design (owner tool, not an end-user surface).

## What's explicitly out of scope for this plan

- Admin panel — not planned for mobile; remains web-only owner tool.
- Any change to `backend/` or firmware for phases 1–4, 7, and 8 — the existing REST contract is reused unchanged.
