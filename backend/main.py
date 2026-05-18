"""FastAPI entrypoint for the AquaFeed backend adapter."""

from datetime import datetime
import os

from fastapi import Depends, FastAPI
from fastapi.middleware.cors import CORSMiddleware

try:
    from .auth import router as auth_router
    from .devices import router as devices_router
    from .config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION, MOCK_DEVICE
    from .dependencies import UserClaims, get_current_user
    from .device_client import close_http_client, request_firmware
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
    from auth import router as auth_router
    from devices import router as devices_router
    from config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION, MOCK_DEVICE
    from dependencies import UserClaims, get_current_user
    from device_client import close_http_client, request_firmware
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

app = FastAPI(
    title="AquaFeed API",
    description="API for controlling the automatic fish feeder",
    version=APP_VERSION,
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


@app.on_event("shutdown")
async def shutdown_event():
    await close_http_client()


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
async def get_status(current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_status()
    response = await request_firmware("/api/status")
    return map_firmware_status(response.json())


@app.post("/api/feed", response_model=CommandResponse)
async def feed_now(request: FeedRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_feed(request.repeats)
    await request_firmware("/api/feedNow", method="POST", data={"repeats": request.repeats})
    return CommandResponse(success=True, message=f"Feeding started with {request.repeats} repeats")


@app.post("/api/speed", response_model=CommandResponse)
async def set_speed(request: SpeedRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_speed(request.speed)
    await request_firmware("/api/setSpeed", method="POST", data={"speed": request.speed})
    return CommandResponse(success=True, message=f"Speed set to {request.speed}")


@app.post("/api/schedule", response_model=CommandResponse)
async def set_schedule(request: ScheduleRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_schedule(request.times)
    await request_firmware("/api/setFeedTimes", method="POST", data={"data": encode_schedule(request.times)})
    return CommandResponse(success=True, message="Schedule updated")


@app.post("/api/angle", response_model=CommandResponse)
async def set_angle(request: AngleRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_angle(request.angle)
    await request_firmware("/api/setAngle", method="POST", data={"angle": request.angle})
    return CommandResponse(success=True, message=f"Angle set to {request.angle}")


@app.post("/api/power-mode", response_model=CommandResponse)
async def set_power_mode(request: PowerModeRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_power_mode(request.enabled)
    await request_firmware(
        "/api/setPowerMode",
        method="POST",
        data={"enabled": str(request.enabled).lower()},
    )
    return CommandResponse(
        success=True,
        message=f"Power save mode {'enabled' if request.enabled else 'disabled'}",
    )


@app.post("/api/display-settings", response_model=CommandResponse)
async def set_display_settings(request: DisplaySettingsRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_display_settings(
            request.powerSaveMode, request.deepSleepIdleSec,
            request.displayEnabled, request.displayOffAfterSec,
        )
    await request_firmware("/api/setDisplaySettings", method="POST", data=request.model_dump())
    return CommandResponse(success=True, message="Display settings updated")


@app.post("/api/min-interval", response_model=CommandResponse)
async def set_min_interval(request: MinIntervalRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_min_interval(request.minFeedIntervalMin)
    await request_firmware("/api/setMinInterval", method="POST", data={"minFeedIntervalMin": request.minFeedIntervalMin})
    return CommandResponse(success=True, message=f"Min interval set to {request.minFeedIntervalMin} min")


@app.post("/api/calibrate", response_model=CommandResponse)
async def calibrate_battery(request: CalibrateRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_calibrate(request.actualVoltage)
    await request_firmware("/api/calibrate", method="POST", data={"actualVoltage": request.actualVoltage})
    return CommandResponse(success=True, message=f"Battery calibrated to {request.actualVoltage} V")


@app.post("/api/timezone", response_model=CommandResponse)
async def set_timezone(request: TimezoneRequest, current_user: UserClaims = Depends(get_current_user)):
    if MOCK_DEVICE:
        return mock_set_timezone(request.offsetHours)
    await request_firmware("/api/setTimezone", method="POST", data={"offsetHours": request.offsetHours})
    return CommandResponse(success=True, message=f"Timezone set to UTC{request.offsetHours:+d}")


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=int(os.getenv("PORT", "8000")))
