"""Pydantic models for the public backend API."""

from pydantic import BaseModel, Field


class FeedRequest(BaseModel):
    repeats: int = Field(default=1, ge=1, le=10, description="Number of feeding repeats")


class SpeedRequest(BaseModel):
    speed: int = Field(ge=1, le=100, description="Servo speed")


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


class StatusResponse(BaseModel):
    angle: int
    speed: int
    feedRepeats: int
    powerSaveMode: bool
    displayEnabled: bool = True
    displayOffAfterSec: int = 20
    deepSleepIdleSec: int = 60
    batteryVoltage: float
    batteryPercent: int
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


class CommandResponse(BaseModel):
    success: bool
    message: str
