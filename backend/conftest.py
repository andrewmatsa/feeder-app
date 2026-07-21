import os
import sys
from contextlib import contextmanager

import pytest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

_MISSING = object()


@contextmanager
def override_dependency(app, dependency, value):
    """Temporarily override a FastAPI dependency, restoring whatever override
    (if any) was already registered — never leave the shared `app` singleton
    with a wiped-out override other test modules rely on."""
    previous = app.dependency_overrides.get(dependency, _MISSING)
    app.dependency_overrides[dependency] = value
    try:
        yield
    finally:
        if previous is _MISSING:
            app.dependency_overrides.pop(dependency, None)
        else:
            app.dependency_overrides[dependency] = previous


def pytest_addoption(parser):
    parser.addoption(
        "--device-url",
        default=None,
        help="Base URL of the real ESP32 device, e.g. http://192.168.1.100",
    )


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "mock: tests that run against the mock device — no hardware needed, safe for CI",
    )
    config.addinivalue_line(
        "markers",
        "hardware: tests that require a real ESP32 on the network — pass --device-url",
    )


@pytest.fixture(scope="session")
def device_url(request):
    url = request.config.getoption("--device-url")
    if not url:
        pytest.skip("Pass --device-url http://<esp32-ip> to run hardware tests")
    return url.rstrip("/")
