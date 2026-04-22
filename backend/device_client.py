"""HTTP client for the ESP32 firmware API."""

from __future__ import annotations

from typing import Any

import httpx
from fastapi import HTTPException

try:
    from .config import ESP32_BASE_URL
except ImportError:
    from config import ESP32_BASE_URL

http_client = httpx.AsyncClient(timeout=5.0, follow_redirects=False)
MUTATION_HEADERS = {"X-AquaFeed-Client": "backend"}


async def request_firmware(
    path: str,
    *,
    method: str = "GET",
    params: dict[str, Any] | None = None,
    data: dict[str, Any] | None = None,
) -> httpx.Response:
    try:
        response = await http_client.request(
            method,
            f"{ESP32_BASE_URL}{path}",
            params=params,
            data=data,
            headers=MUTATION_HEADERS if method.upper() != "GET" else None,
        )
        response.raise_for_status()
        return response
    except httpx.TimeoutException as exc:
        raise HTTPException(status_code=504, detail="ESP32 request timed out") from exc
    except httpx.HTTPStatusError as exc:
        raise HTTPException(
            status_code=502,
            detail=f"ESP32 rejected the request: {exc.response.text or exc.response.reason_phrase}",
        ) from exc
    except httpx.RequestError as exc:
        raise HTTPException(status_code=503, detail=f"Failed to communicate with ESP32: {exc}") from exc


async def close_http_client() -> None:
    await http_client.aclose()
