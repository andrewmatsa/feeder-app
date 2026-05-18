"""Mock device responses for development without a physical ESP32."""

from datetime import datetime

try:
    from .models import CommandResponse, FeedTime, StatusResponse
except ImportError:
    from models import CommandResponse, FeedTime, StatusResponse

_feed_times = [
    FeedTime(hour=8, minute=0, repeats=1),
    FeedTime(hour=13, minute=30, repeats=2),
    FeedTime(hour=19, minute=0, repeats=1),
]

_state = {
    "angle": 90,
    "speed": 50,
    "feedRepeats": 1,
    "powerSaveMode": False,
    "batteryVoltage": 3.85,
    "batteryPercent": 72,
    "isCharging": True,
    "wifiSSID": "HomeNetwork",
    "wifiIP": "192.168.1.42",
    "deepSleepIdleSec": 120,
    "displayEnabled": True,
    "displayOffAfterSec": 30,
    "minFeedIntervalMin": 5,
    "timezoneOffset": 2,
}


def mock_status() -> StatusResponse:
    now = datetime.now()
    return StatusResponse(
        angle=_state["angle"],
        speed=_state["speed"],
        feedRepeats=_state["feedRepeats"],
        minFeedIntervalMin=_state["minFeedIntervalMin"],
        powerSaveMode=_state["powerSaveMode"],
        displayEnabled=_state["displayEnabled"],
        displayOffAfterSec=_state["displayOffAfterSec"],
        deepSleepIdleSec=_state["deepSleepIdleSec"],
        batteryVoltage=_state["batteryVoltage"],
        batteryPercent=_state["batteryPercent"],
        isCharging=_state["isCharging"],
        feedTimes=_feed_times,
        nextFeedHour=13,
        nextFeedMinute=30,
        nextFeedMinutes=((13 * 60 + 30) - (now.hour * 60 + now.minute)) % (24 * 60),
        currentTime=now.strftime("%H:%M:%S"),
        manualFeedCooldownSeconds=0,
        wifiSSID=_state["wifiSSID"],
        wifiIP=_state["wifiIP"],
        isAPMode=False,
        sleepReason="none",
        sleepCountdownSeconds=-1,
        displayAwake=True,
        timestamp=now.isoformat(),
        cpuFrequency=160,
        memoryFreeHeap=245320,
        memoryUsedHeap=154680,
        memoryTotalHeap=400000,
        memoryMaxAllocHeap=180500,
        memoryMinFreeHeap=200250,
        cacheSize=500,
        cacheAge=245,
        cacheValid=True,
        uptimeSeconds=8100,
    )


def mock_feed(repeats: int) -> CommandResponse:
    return CommandResponse(success=True, message=f"[MOCK] Feeding started with {repeats} repeats")


def mock_set_speed(speed: int) -> CommandResponse:
    _state["speed"] = speed
    return CommandResponse(success=True, message=f"[MOCK] Speed set to {speed}")


def mock_set_angle(angle: int) -> CommandResponse:
    _state["angle"] = angle
    return CommandResponse(success=True, message=f"[MOCK] Angle set to {angle}")


def mock_set_schedule(times: list[FeedTime]) -> CommandResponse:
    global _feed_times
    _feed_times = times
    return CommandResponse(success=True, message=f"[MOCK] Schedule updated with {len(times)} slots")


def mock_set_power_mode(enabled: bool) -> CommandResponse:
    _state["powerSaveMode"] = enabled
    return CommandResponse(success=True, message=f"[MOCK] Power save {'enabled' if enabled else 'disabled'}")


def mock_set_display_settings(power_save: bool, deep_sleep_sec: int, display_enabled: bool, display_off_sec: int) -> CommandResponse:
    _state["powerSaveMode"] = power_save
    _state["deepSleepIdleSec"] = deep_sleep_sec
    _state["displayEnabled"] = display_enabled
    _state["displayOffAfterSec"] = display_off_sec
    return CommandResponse(success=True, message="[MOCK] Display settings updated")


def mock_set_min_interval(min_interval_min: int) -> CommandResponse:
    _state["minFeedIntervalMin"] = min_interval_min
    return CommandResponse(success=True, message=f"[MOCK] Min interval set to {min_interval_min} min")


def mock_calibrate(actual_voltage: float) -> CommandResponse:
    _state["batteryVoltage"] = actual_voltage
    return CommandResponse(success=True, message=f"[MOCK] Battery calibrated to {actual_voltage} V")


def mock_set_timezone(offset_hours: int) -> CommandResponse:
    _state["timezoneOffset"] = offset_hours
    return CommandResponse(success=True, message=f"[MOCK] Timezone set to UTC{offset_hours:+d}")
