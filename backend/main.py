"""FastAPI entrypoint for the AquaFeed backend adapter."""

from datetime import datetime
import os

from fastapi import Depends, FastAPI
from fastapi.middleware.cors import CORSMiddleware
from gotrue.types import User

try:
    from .auth import router as auth_router
    from .config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION
    from .dependencies import get_current_user
    from .device_client import close_http_client, request_firmware
    from .mappers import encode_schedule, map_firmware_status
    from .models import (
        AngleRequest,
        CommandResponse,
        FeedRequest,
        PowerModeRequest,
        ScheduleRequest,
        SpeedRequest,
        StatusResponse,
    )
except ImportError:
    from auth import router as auth_router
    from config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION
    from dependencies import get_current_user
    from device_client import close_http_client, request_firmware
    from mappers import encode_schedule, map_firmware_status
    from models import (
        AngleRequest,
        CommandResponse,
        FeedRequest,
        PowerModeRequest,
        ScheduleRequest,
        SpeedRequest,
        StatusResponse,
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
async def get_status(current_user: User = Depends(get_current_user)):
    response = await request_firmware("/api/status")
    return map_firmware_status(response.json())


@app.post("/api/feed", response_model=CommandResponse)
async def feed_now(request: FeedRequest, current_user: User = Depends(get_current_user)):
    await request_firmware("/api/feedNow", method="POST", data={"repeats": request.repeats})
    return CommandResponse(
        success=True,
        message=f"Feeding started with {request.repeats} repeats",
    )


@app.post("/api/speed", response_model=CommandResponse)
async def set_speed(request: SpeedRequest, current_user: User = Depends(get_current_user)):
    await request_firmware("/api/setSpeed", method="POST", data={"speed": request.speed})
    return CommandResponse(success=True, message=f"Speed set to {request.speed}")


@app.post("/api/schedule", response_model=CommandResponse)
async def set_schedule(request: ScheduleRequest, current_user: User = Depends(get_current_user)):
    await request_firmware("/api/setFeedTimes", method="POST", data={"data": encode_schedule(request.times)})
    return CommandResponse(success=True, message="Schedule updated")


@app.post("/api/angle", response_model=CommandResponse)
async def set_angle(request: AngleRequest, current_user: User = Depends(get_current_user)):
    await request_firmware("/api/setAngle", method="POST", data={"angle": request.angle})
    return CommandResponse(success=True, message=f"Angle set to {request.angle}")


@app.post("/api/power-mode", response_model=CommandResponse)
async def set_power_mode(request: PowerModeRequest, current_user: User = Depends(get_current_user)):
    await request_firmware(
        "/api/setPowerMode",
        method="POST",
        data={"enabled": str(request.enabled).lower()},
    )
    return CommandResponse(
        success=True,
        message=f"Power save mode {'enabled' if request.enabled else 'disabled'}",
    )


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=int(os.getenv("PORT", "8000")))
