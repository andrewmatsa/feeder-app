"""
FastAPI backend for AquaFeed - Automatic Fish Feeder
"""
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime
import httpx
import os
from dotenv import load_dotenv

load_dotenv()

app = FastAPI(
    title="AquaFeed API",
    description="API for controlling the automatic fish feeder",
    version="1.0.0"
)

# CORS settings
app.add_middleware(
    CORSMiddleware,
    allow_origins=os.getenv("CORS_ORIGINS", "http://localhost:5173,http://localhost:3000").split(","),
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ESP32 URL (can be configured via environment variables)
ESP32_BASE_URL = os.getenv("ESP32_BASE_URL", "http://fish-eat.local")

# Data models
class FeedRequest(BaseModel):
    repeats: int = Field(default=1, ge=1, le=10, description="Number of feeding repeats")

class SpeedRequest(BaseModel):
    speed: int = Field(ge=1, le=100, description="Servo speed")

class ScheduleRequest(BaseModel):
    times: List[str] = Field(description="List of feeding times in HH:MM format")

class AngleRequest(BaseModel):
    angle: int = Field(ge=0, le=180, description="Servo angle")

class PowerModeRequest(BaseModel):
    enabled: bool = Field(description="Enable/disable power saving mode")

class StatusResponse(BaseModel):
    angle: int
    speed: int
    feedRepeats: int
    powerSaveMode: bool
    batteryVoltage: float
    batteryPercent: int
    feedTimes: List[str]
    timestamp: Optional[str] = None

# HTTP client for ESP32 requests
http_client = httpx.AsyncClient(timeout=5.0)

@app.on_event("shutdown")
async def shutdown_event():
    await http_client.aclose()

@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "message": "AquaFeed API",
        "version": "1.0.0",
        "status": "running"
    }

@app.get("/api/status", response_model=StatusResponse)
async def get_status():
    """
    Get current system status.
    Can work as an ESP32 proxy or return local fallback data.
    """
    try:
        # Try to fetch data from ESP32
        response = await http_client.get(f"{ESP32_BASE_URL}/api/status")
        if response.status_code == 200:
            data = response.json()
            data["timestamp"] = datetime.now().isoformat()
            return StatusResponse(**data)
    except (httpx.RequestError, httpx.TimeoutException):
        # If ESP32 is unavailable, return an error or default values
        pass
    
    # Return an error if ESP32 is unavailable
    raise HTTPException(
        status_code=503,
        detail="ESP32 device is not available. Please check connection."
    )

@app.post("/api/feed")
async def feed_now(request: FeedRequest):
    """Manual feeding"""
    try:
        response = await http_client.post(
            f"{ESP32_BASE_URL}/api/feed",
            json={"repeats": request.repeats}
        )
        response.raise_for_status()
        return {"success": True, "message": f"Feeding started with {request.repeats} repeats"}
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {str(e)}")

@app.post("/api/speed")
async def set_speed(request: SpeedRequest):
    """Set servo speed"""
    try:
        response = await http_client.post(
            f"{ESP32_BASE_URL}/api/speed",
            json={"speed": request.speed}
        )
        response.raise_for_status()
        return {"success": True, "speed": request.speed}
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {str(e)}")

@app.post("/api/schedule")
async def set_schedule(request: ScheduleRequest):
    """Set feeding schedule"""
    try:
        response = await http_client.post(
            f"{ESP32_BASE_URL}/api/schedule",
            json={"times": request.times}
        )
        response.raise_for_status()
        return {"success": True, "times": request.times}
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {str(e)}")

@app.post("/api/angle")
async def set_angle(request: AngleRequest):
    """Set servo angle"""
    try:
        response = await http_client.post(
            f"{ESP32_BASE_URL}/api/angle",
            json={"angle": request.angle}
        )
        response.raise_for_status()
        return {"success": True, "angle": request.angle}
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {str(e)}")

@app.post("/api/power-mode")
async def set_power_mode(request: PowerModeRequest):
    """Set power saving mode"""
    try:
        response = await http_client.post(
            f"{ESP32_BASE_URL}/api/power-mode",
            json={"enabled": request.enabled}
        )
        response.raise_for_status()
        return {"success": True, "powerSaveMode": request.enabled}
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {str(e)}")

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "timestamp": datetime.now().isoformat()}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

