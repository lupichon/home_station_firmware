"""
Copy the firmware binary into a dedicated "firmwares" folder after building,
naming it with the current firmware version (read from
src/core/firmware_version.hpp).
"""

import os
import re

Import("env")

VERSION_HEADER_PATH = os.path.join("src", "core", "firmware_version.hpp")
OUTPUT_DIR = "firmwares"


def read_version_string():
    if not os.path.isfile(VERSION_HEADER_PATH):
        return "0.0.0"

    with open(VERSION_HEADER_PATH, "r") as f:
        content = f.read()

    match = re.search(r'FIRMWARE_VERSION_STRING\s+"([^"]+)"', content)
    return match.group(1) if match else "0.0.0"


def copy_firmware(source, target, env):
    version = read_version_string()
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    dest_path = os.path.join(OUTPUT_DIR, f"firmware_v{version}.bin")

    env.Execute(
        env.VerboseAction(
            f"cp $BUILD_DIR/firmware.bin {dest_path}",
            f"Copying firmware.bin -> {dest_path}"
        )
    )


env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)