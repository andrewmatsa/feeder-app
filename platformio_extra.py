Import("env")

import json
from pathlib import Path


project_dir = Path(env["PROJECT_DIR"])
versions = json.loads((project_dir / "versions.json").read_text(encoding="utf-8"))

env.Append(
    CPPDEFINES=[
        ("AQUAFEED_APP_VERSION", '\\"{}\\"'.format(versions["appVersion"])),
        ("AQUAFEED_FIRMWARE_VERSION", '\\"{}\\"'.format(versions["firmwareVersion"])),
    ]
)
