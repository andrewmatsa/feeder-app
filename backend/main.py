"""FastAPI entrypoint for the AquaFeed backend adapter."""

from datetime import datetime
import os

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

try:
    from .config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION
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
    from config import APP_VERSION, CORS_ORIGINS, FIRMWARE_VERSION
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

# CORS settings
app.add_middleware(
    CORSMiddleware,
    allow_origins=CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.on_event("shutdown")
async def shutdown_event():
    await close_http_client()

@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "message": "AquaFeed API",
        "version": APP_VERSION,
        "firmwareVersion": FIRMWARE_VERSION,
        "status": "running",
    }

@app.get("/api/status", response_model=StatusResponse)
async def get_status():
    response = await request_firmware("/api/status")
    return map_firmware_status(response.json())


@app.post("/api/feed", response_model=CommandResponse)
async def feed_now(request: FeedRequest):
    await request_firmware("/api/setRepeats", params={"repeats": request.repeats})
    await request_firmware("/api/feedNow")
    return CommandResponse(
        success=True,
        message=f"Feeding started with {request.repeats} repeats",
    )


@app.post("/api/speed", response_model=CommandResponse)
async def set_speed(request: SpeedRequest):
    await request_firmware("/api/setSpeed", params={"speed": request.speed})
    return CommandResponse(success=True, message=f"Speed set to {request.speed}")


@app.post("/api/schedule", response_model=CommandResponse)
async def set_schedule(request: ScheduleRequest):
    await request_firmware("/api/setFeedTimes", params={"data": encode_schedule(request.times)})
    return CommandResponse(success=True, message="Schedule updated")


@app.post("/api/angle", response_model=CommandResponse)
async def set_angle(request: AngleRequest):
    await request_firmware("/api/setAngle", params={"angle": request.angle})
    return CommandResponse(success=True, message=f"Angle set to {request.angle}")


@app.post("/api/power-mode", response_model=CommandResponse)
async def set_power_mode(request: PowerModeRequest):
    await request_firmware(
        "/api/setPowerMode",
        params={"enabled": str(request.enabled).lower()},
    )
    return CommandResponse(
        success=True,
        message=f"Power save mode {'enabled' if request.enabled else 'disabled'}",
    )

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "timestamp": datetime.now().isoformat()}

if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=int(os.getenv("PORT", "8000")))

