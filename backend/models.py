"""Pydantic models for the public backend API."""

from pydantic import BaseModel, Field


class FeedRequest(BaseModel):
    repeats: int = Field(default=1, ge=1, le=10, description="Number of feeding repeats")


class SpeedRequest(BaseModel):
    speed: float = Field(ge=1.0, le=20.0, description="Servo speed")


class FeedTime(BaseModel):
    hour: int = Field(ge=0, le=23, description="Feeding hour")
    minute: int = Field(ge=0, le=59, description="Feeding minute")
    repeats: int = Field(default=1, ge=1, le=20, description="Feed repeats for this slot")


class ScheduleRequest(BaseModel):
    times: list[FeedTime] = Field(description="List of feeding times")


class AngleRequest(BaseModel):
    angle: int = Field(ge=0, le=180, description="Servo angle")


class PowerModeRequest(BaseModel):
    enabled: bool = Field(description="Enable or disable power saving mode")


class DisplaySettingsRequest(BaseModel):
    powerSaveMode: bool
    deepSleepIdleSec: int = Field(ge=10, le=3600)
    displayEnabled: bool
    displayOffAfterSec: int = Field(ge=5, le=300)


class MinIntervalRequest(BaseModel):
    minFeedIntervalMin: int = Field(ge=1, le=1440)


class CalibrateRequest(BaseModel):
    actualVoltage: float = Field(ge=2.5, le=4.5)


class TimezoneRequest(BaseModel):
    offsetHours: int = Field(ge=-12, le=14)


class StatusResponse(BaseModel):
    angle: int
    speed: float
    feedRepeats: int
    minFeedIntervalMin: int = 5
    powerSaveMode: bool
    displayEnabled: bool = True
    displayOffAfterSec: int = 20
    deepSleepIdleSec: int = 60
    batteryVoltage: float
    batteryPercent: int
    isCharging: bool = False
    feedTimes: list[FeedTime]
    nextFeedMinutes: int | None = None
    nextFeedHour: int | None = None
    nextFeedMinute: int | None = None
    currentTime: str | None = None
    manualFeedCooldownSeconds: int = 0
    wifiSSID: str = ""
    wifiIP: str = ""
    isAPMode: bool = False
    sleepReason: str = "unknown"
    sleepCountdownSeconds: int = -1
    displayAwake: bool = True
    timestamp: str
    # System info
    cpuFrequency: int | None = None
    memoryFreeHeap: int | None = None
    memoryUsedHeap: int | None = None
    memoryTotalHeap: int | None = None
    memoryMaxAllocHeap: int | None = None
    memoryMinFreeHeap: int | None = None
    cacheSize: int | None = None
    cacheAge: int | None = None
    cacheValid: bool | None = None
    uptimeSeconds: int | None = None
    # Light sensor
    lightLux: int | None = None


class CommandResponse(BaseModel):
    success: bool
    message: str


class DeviceResponse(BaseModel):
    id: str
    name: str
    macAddress: str | None = None
    createdAt: str
    lastSeen: str | None = None


class CreateDeviceRequest(BaseModel):
    name: str | None = Field(
        default=None,
        min_length=2,
        max_length=40,
        description="Display name for the aquarium, e.g. Вітальня",
    )
    macAddress: str | None = None


class UpdateDeviceRequest(BaseModel):
    name: str | None = Field(default=None, min_length=2, max_length=40)
