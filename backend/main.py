"""FastAPI entrypoint for the AquaFeed backend adapter."""

from datetime import datetime
import os

from contextlib import asynccontextmanager

from fastapi import Depends, FastAPI, File, HTTPException, UploadFile
from fastapi.middleware.cors import CORSMiddleware

try:
    from .admin import router as admin_router, simulated_offline_devices
    from .auth import router as auth_router
    from .devices import router as devices_router
    from .mock_realtime import mock_record_feed, mock_touch_last_seen
    from .config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION, MOCK_DEVICE
    from .dependencies import UserClaims, get_current_user
    from .device_client import close_http_client, get_device_base_url, request_firmware
    from .mappers import encode_schedule, map_firmware_status
    from .mock_device import (
        mock_calibrate, mock_feed, mock_set_angle, mock_set_display_settings,
        mock_set_min_interval, mock_set_power_mode, mock_set_schedule,
        mock_set_speed, mock_set_timezone, mock_status,
    )
    from .models import (
        AngleRequest,
        CalibrateRequest,
        CommandResponse,
        DisplaySettingsRequest,
        FeedRequest,
        MinIntervalRequest,
        PowerModeRequest,
        ScheduleRequest,
        SpeedRequest,
        StatusResponse,
        TimezoneRequest,
    )
except ImportError:
    from admin import router as admin_router, simulated_offline_devices
    from auth import router as auth_router
    from devices import router as devices_router
    from mock_realtime import mock_record_feed, mock_touch_last_seen
    from config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION, MOCK_DEVICE
    from dependencies import UserClaims, get_current_user
    from device_client import close_http_client, get_device_base_url, request_firmware
    from mappers import encode_schedule, map_firmware_status
    from mock_device import (
        mock_calibrate, mock_feed, mock_set_angle, mock_set_display_settings,
        mock_set_min_interval, mock_set_power_mode, mock_set_schedule,
        mock_set_speed, mock_set_timezone, mock_status,
    )
    from models import (
        AngleRequest,
        CalibrateRequest,
        CommandResponse,
        DisplaySettingsRequest,
        FeedRequest,
        MinIntervalRequest,
        PowerModeRequest,
        ScheduleRequest,
        SpeedRequest,
        StatusResponse,
        TimezoneRequest,
    )

@asynccontextmanager
async def lifespan(app: FastAPI):
    yield
    await close_http_client()


app = FastAPI(
    title="AquaFeed API",
    description="API for controlling the automatic fish feeder",
    version=APP_VERSION,
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(auth_router)
app.include_router(devices_router)
app.include_router(admin_router)


@app.get("/")
async def root():
    return {
        "message": "AquaFeed API",
        "version": APP_VERSION,
        "firmwareVersion": FIRMWARE_VERSION,
        "status": "running",
    }


@app.get("/health")
async def health_check():
    return {"status": "healthy", "timestamp": datetime.now().isoformat()}


@app.get("/api/status", response_model=StatusResponse)
async def get_status(
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if device_id and device_id in simulated_offline_devices:
        raise HTTPException(status_code=503, detail="Device offline (simulated)")
    if MOCK_DEVICE:
        mock_touch_last_seen(current_user.id, device_id)
        return mock_status(device_id)
    response = await request_firmware("/api/status", base_url=get_device_base_url(device_id))
    return map_firmware_status(response.json())


@app.post("/api/feed", response_model=CommandResponse)
async def feed_now(
    request: FeedRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        mock_record_feed(current_user.id, request.repeats, device_id)
        return mock_feed(request.repeats)
    await request_firmware(
        "/api/feedNow", method="POST", data={"repeats": request.repeats},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Feeding started with {request.repeats} repeats")


@app.post("/api/speed", response_model=CommandResponse)
async def set_speed(
    request: SpeedRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_speed(request.speed, device_id)
    await request_firmware(
        "/api/setSpeed", method="POST", data={"speed": request.speed},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Speed set to {request.speed}")


@app.post("/api/schedule", response_model=CommandResponse)
async def set_schedule(
    request: ScheduleRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_schedule(request.times, device_id)
    await request_firmware(
        "/api/setFeedTimes", method="POST", data={"data": encode_schedule(request.times)},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message="Schedule updated")


@app.post("/api/angle", response_model=CommandResponse)
async def set_angle(
    request: AngleRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_angle(request.angle, device_id)
    await request_firmware(
        "/api/setAngle", method="POST", data={"angle": request.angle},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Angle set to {request.angle}")


@app.post("/api/power-mode", response_model=CommandResponse)
async def set_power_mode(
    request: PowerModeRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_power_mode(request.enabled, device_id)
    await request_firmware(
        "/api/setPowerMode",
        method="POST",
        data={"enabled": str(request.enabled).lower()},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(
        success=True,
        message=f"Power save mode {'enabled' if request.enabled else 'disabled'}",
    )


@app.post("/api/display-settings", response_model=CommandResponse)
async def set_display_settings(
    request: DisplaySettingsRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_display_settings(
            request.powerSaveMode, request.deepSleepIdleSec,
            request.displayEnabled, request.displayOffAfterSec,
            device_id,
        )
    base = get_device_base_url(device_id)
    await request_firmware(
        "/api/setPowerMode", method="POST",
        data={"enabled": str(request.powerSaveMode).lower(), "idleSec": request.deepSleepIdleSec},
        base_url=base,
    )
    await request_firmware(
        "/api/setDisplayMode", method="POST",
        data={"enabled": str(request.displayEnabled).lower(), "sec": request.displayOffAfterSec},
        base_url=base,
    )
    return CommandResponse(success=True, message="Display settings updated")


@app.post("/api/min-interval", response_model=CommandResponse)
async def set_min_interval(
    request: MinIntervalRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_min_interval(request.minFeedIntervalMin, device_id)
    await request_firmware(
        "/api/setMinInterval", method="POST", data={"minFeedIntervalMin": request.minFeedIntervalMin},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Min interval set to {request.minFeedIntervalMin} min")


@app.post("/api/calibrate", response_model=CommandResponse)
async def calibrate_battery(
    request: CalibrateRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_calibrate(request.actualVoltage, device_id)
    await request_firmware(
        "/api/calibrate", method="POST", data={"actualVoltage": request.actualVoltage},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Battery calibrated to {request.actualVoltage} V")


@app.post("/api/timezone", response_model=CommandResponse)
async def set_timezone(
    request: TimezoneRequest,
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return mock_set_timezone(request.offsetHours, device_id)
    await request_firmware(
        "/api/setTimezone", method="POST", data={"offsetHours": request.offsetHours},
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message=f"Timezone set to UTC{request.offsetHours:+d}")


@app.post("/api/forget-wifi", response_model=CommandResponse)
async def forget_wifi(
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return CommandResponse(success=True, message="WiFi credentials cleared (mock)")
    await request_firmware(
        "/api/forgetWiFi", method="POST",
        base_url=get_device_base_url(device_id),
    )
    return CommandResponse(success=True, message="WiFi credentials cleared")


@app.post("/api/ota-update", response_model=CommandResponse)
async def ota_update(
    file: UploadFile = File(...),
    current_user: UserClaims = Depends(get_current_user),
    device_id: str | None = None,
):
    if MOCK_DEVICE:
        return CommandResponse(success=True, message="OTA skipped (mock device)")

    if not file.filename or not file.filename.endswith(".bin"):
        raise HTTPException(status_code=400, detail="File must be a .bin firmware image")

    import httpx
    base_url = get_device_base_url(device_id)

    async def _gen():
        while chunk := await file.read(8192):
            yield chunk

    try:
        async with httpx.AsyncClient(timeout=120.0) as ota_client:
            response = await ota_client.post(
                f"{base_url}/api/otaUpdate",
                headers={"X-AquaFeed-Client": "backend"},
                content=_gen(),
            )
            response.raise_for_status()
    except httpx.TimeoutException as exc:
        raise HTTPException(status_code=504, detail="OTA timed out") from exc
    except httpx.HTTPStatusError as exc:
        raise HTTPException(status_code=502, detail=f"OTA failed: {exc.response.text}") from exc
    except httpx.RequestError as exc:
        raise HTTPException(status_code=503, detail=f"OTA connection error: {exc}") from exc

    return CommandResponse(success=True, message="Firmware update complete, device rebooting")


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=int(os.getenv("PORT", "8000")))
