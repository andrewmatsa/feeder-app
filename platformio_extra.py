Import("env")

import json
import shutil
from pathlib import Path
from datetime import datetime

project_dir = Path(env["PROJECT_DIR"])
versions = json.loads((project_dir / "versions.json").read_text(encoding="utf-8"))

now = datetime.now()
firmware_version = versions["firmwareVersion"]
build_date = now.strftime("%Y-%m-%d")

env.Append(
    CPPDEFINES=[
        ("AQUAFEED_APP_VERSION", '\\"{}\\"'.format(versions["appVersion"])),
        ("AQUAFEED_FIRMWARE_VERSION", '\\"{}\\"'.format(firmware_version)),
        ("BUILD_DATE", '\\"{}\\"'.format(build_date)),
        ("BUILD_TIME", '\\"{}\\"'.format(now.strftime("%H:%M:%S"))),
    ]
)


def rename_firmware(source, target, env):
    build_dir = Path(env.subst("$BUILD_DIR"))
    src = build_dir / "firmware.bin"
    dst = build_dir / "aquafeed-v{}-{}.bin".format(firmware_version, build_date)
    if src.exists():
        shutil.copy2(str(src), str(dst))
        print("\n*** Firmware binary: {} ***\n".format(dst.name))


env.AddPostAction("buildprog", rename_firmware)
