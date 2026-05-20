#!/usr/bin/env python3
"""Hardware integration test script for AquaFeed ESP32.

Usage:
  python scripts/test_device.py --url http://192.168.x.x       # real ESP32
  python scripts/test_device.py --url http://192.168.x.x --skip-servo  # skip servo moves
  python scripts/test_device.py --url http://localhost:8000     # backend proxy (no servo test)

The script sends HTTP requests directly to the ESP32 REST API and reports
which checks pass or fail. Exit code 0 = all pass, 1 = some fail.
"""
from __future__ import annotations

import argparse
import sys
import time

try:
    import requests
except ImportError:
    print("ERROR: 'requests' not installed. Run: pip install requests")
    sys.exit(2)

AUTH_HEADER = {"X-AquaFeed-Client": "backend"}

_passed = 0
_failed = 0


def ok(name: str) -> None:
    global _passed
    _passed += 1
    print(f"  \033[32m✓\033[0m {name}")


def fail(name: str, detail: str = "") -> None:
    global _failed
    _failed += 1
    msg = f"  \033[31m✗\033[0m {name}"
    if detail:
        msg += f"\n      {detail}"
    print(msg)


def check(name: str, condition: bool, detail: str = "") -> bool:
    if condition:
        ok(name)
        return True
    else:
        fail(name, detail)
        return False


def run(base_url: str, skip_servo: bool = False) -> None:
    base = base_url.rstrip("/")
    via_proxy = "8000" in base or "/api" in base

    print(f"\n{'='*50}")
    print(f"AquaFeed Device Tests")
    print(f"Target : {base}")
    print(f"Mode   : {'backend proxy (servo test skipped)' if via_proxy else 'direct ESP32'}")
    print(f"{'='*50}\n")

    # ── 1. Root endpoint ──────────────────────────────────────────────────────
    print("[ GET / ]")
    try:
        r = requests.get(f"{base}/", timeout=5)
        body = r.json()
        check("HTTP 200", r.status_code == 200, f"got {r.status_code}")
        check("Content-Type is JSON", "application/json" in r.headers.get("Content-Type", ""),
              r.headers.get("Content-Type", ""))
        check("'device' key present", "device" in body, f"keys: {list(body.keys())}")
        check("'api' key present", "api" in body, f"keys: {list(body.keys())}")
        check("Not HTML", "<html" not in r.text.lower(), "response looks like HTML")
    except Exception as e:
        fail("GET /", str(e))

    # ── 2. Status endpoint ────────────────────────────────────────────────────
    print("\n[ GET /api/status ]")
    status_body: dict = {}
    try:
        r = requests.get(f"{base}/api/status", timeout=5)
        check("HTTP 200", r.status_code == 200, f"got {r.status_code}")
        if r.status_code == 200:
            status_body = r.json()
            required_keys = {
                "status", "batteryVoltage", "batteryPercent",
                "feedTimes", "speed", "currentAngle", "feedRepeats",
                "powerSaveMode", "displayEnabled",
            }
            missing = required_keys - status_body.keys()
            check("All required fields present", not missing, f"missing: {missing}")
            check("feedTimes is a list", isinstance(status_body.get("feedTimes"), list))
            check("No legacy feedHour1 field", "feedHour1" not in status_body)
            check("No legacy feedHour2 field", "feedHour2" not in status_body)
            check("batteryVoltage is a number", isinstance(status_body.get("batteryVoltage"), (int, float)))
    except Exception as e:
        fail("GET /api/status", str(e))

    # ── 3. Auth check for mutations ───────────────────────────────────────────
    print("\n[ POST /api/feedNow — auth check ]")
    try:
        r = requests.post(f"{base}/api/feedNow", timeout=5)  # no auth header
        check("POST without header → 403", r.status_code == 403,
              f"got {r.status_code} (expected 403 — missing client header)")
    except Exception as e:
        fail("auth check", str(e))

    # ── 4. Feed endpoint ──────────────────────────────────────────────────────
    print("\n[ POST /api/feedNow ]")
    if via_proxy:
        print("  (skipped — targeting backend proxy, not raw ESP32)")
    else:
        try:
            r = requests.post(
                f"{base}/api/feedNow",
                headers=AUTH_HEADER,
                data={"repeats": "1"},
                timeout=15,
            )
            check("HTTP 200", r.status_code == 200, f"got {r.status_code}: {r.text}")
            check("Response is 'feeding'", "feeding" in r.text.lower(),
                  f"got: {r.text!r}")
        except Exception as e:
            fail("POST /api/feedNow", str(e))

    # ── 5. Speed slider test (servo timing) ───────────────────────────────────
    print("\n[ Servo speed test ]")
    if via_proxy or skip_servo:
        print("  (skipped — not targeting raw ESP32 or --skip-servo)")
    else:
        try:
            # Set speed=1 (slow), measure feed time
            requests.post(f"{base}/api/setSpeed", headers=AUTH_HEADER,
                          data={"speed": "1"}, timeout=5)
            time.sleep(0.5)
            t0 = time.time()
            requests.post(f"{base}/api/feedNow", headers=AUTH_HEADER,
                          data={"repeats": "1"}, timeout=30)
            slow_time = time.time() - t0

            # Set speed=20 (fast), measure feed time
            requests.post(f"{base}/api/setSpeed", headers=AUTH_HEADER,
                          data={"speed": "20"}, timeout=5)
            time.sleep(0.5)
            t0 = time.time()
            requests.post(f"{base}/api/feedNow", headers=AUTH_HEADER,
                          data={"repeats": "1"}, timeout=30)
            fast_time = time.time() - t0

            ratio = slow_time / fast_time if fast_time > 0 else 0
            check(
                f"Speed=1 slower than speed=20 (ratio {ratio:.1f}×)",
                ratio >= 5,
                f"slow={slow_time:.1f}s  fast={fast_time:.1f}s  ratio={ratio:.1f}× (expected ≥5×)",
            )

            # Restore default speed
            requests.post(f"{base}/api/setSpeed", headers=AUTH_HEADER,
                          data={"speed": "20"}, timeout=5)
        except Exception as e:
            fail("Servo speed test", str(e))

    # ── 6. Schedule round-trip ────────────────────────────────────────────────
    print("\n[ Schedule round-trip ]")
    if via_proxy:
        print("  (skipped — targeting backend proxy)")
    else:
        try:
            import json
            new_schedule = json.dumps([{"h": 7, "m": 30, "r": 2}, {"h": 20, "m": 0, "r": 1}])
            requests.post(f"{base}/api/setFeedTimes", headers=AUTH_HEADER,
                          data={"data": new_schedule}, timeout=5)
            time.sleep(0.2)
            r = requests.get(f"{base}/api/status", timeout=5)
            times = r.json().get("feedTimes", [])
            check("Schedule saved (2 slots)", len(times) == 2,
                  f"got {len(times)} slots")
            if len(times) >= 1:
                check("Slot 0 hour=7", times[0].get("h") == 7 or times[0].get("hour") == 7,
                      f"got {times[0]}")
        except Exception as e:
            fail("Schedule round-trip", str(e))

    # ── 7. Battery rate-limit ─────────────────────────────────────────────────
    print("\n[ Battery rate-limit ]")
    if via_proxy:
        print("  (skipped — targeting backend proxy)")
    else:
        try:
            r1 = requests.get(f"{base}/api/status", timeout=5).json()
            time.sleep(1)  # within 5s rate-limit window
            r2 = requests.get(f"{base}/api/status", timeout=5).json()
            v1 = r1.get("batteryVoltage")
            v2 = r2.get("batteryVoltage")
            check(
                "Battery voltage same within 5s (rate-limit active)",
                v1 == v2,
                f"first={v1}V  second={v2}V — values differ (rate-limit may not be working)",
            )
        except Exception as e:
            fail("Battery rate-limit", str(e))

    # ── Summary ───────────────────────────────────────────────────────────────
    total = _passed + _failed
    print(f"\n{'='*50}")
    print(f"Results: {_passed}/{total} passed", end="")
    if _failed:
        print(f"  \033[31m({_failed} failed)\033[0m")
    else:
        print("  \033[32m(all pass)\033[0m")
    print("="*50)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AquaFeed ESP32 hardware integration tests")
    parser.add_argument("--url", required=True,
                        help="Base URL: http://192.168.x.x (ESP32) or http://localhost:8000 (proxy)")
    parser.add_argument("--skip-servo", action="store_true",
                        help="Skip servo movement tests (speed timing + feedNow)")
    args = parser.parse_args()

    run(args.url, skip_servo=args.skip_servo)
    sys.exit(0 if _failed == 0 else 1)
