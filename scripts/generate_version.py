"""
@file    generate_version.py
@brief   Ensure src/core/firmware_version.hpp exists and optionally bump it
         based on the BUMP environment variable (major | minor | patch).

Usage:
    pio run                    -> no bump; creates the file at 0.0.0 if missing,
                                   otherwise leaves it untouched
    BUMP=patch pio run         -> patch += 1
    BUMP=minor pio run         -> minor += 1, patch = 0
    BUMP=major pio run         -> major += 1, minor = 0, patch = 0
"""

import os
import re

Import("env")

HEADER_PATH = os.path.join("src", "core", "firmware_version.hpp")


def read_existing_version():
    if not os.path.isfile(HEADER_PATH):
        return 0, 0, 0

    with open(HEADER_PATH, "r") as f:
        content = f.read()

    major = re.search(r"FIRMWARE_VERSION_MAJOR\s+(\d+)", content)
    minor = re.search(r"FIRMWARE_VERSION_MINOR\s+(\d+)", content)
    patch = re.search(r"FIRMWARE_VERSION_PATCH\s+(\d+)", content)

    return (
        int(major.group(1)) if major else 0,
        int(minor.group(1)) if minor else 0,
        int(patch.group(1)) if patch else 0,
    )


def apply_bump(major, minor, patch, bump):
    if bump == "major":
        return major + 1, 0, 0
    if bump == "minor":
        return major, minor + 1, 0
    if bump == "patch":
        return major, minor, patch + 1
    return major, minor, patch


def write_header(major, minor, patch):
    content = f"""// firmware_version.hpp
// Auto-managed by scripts/generate_version.py.
// Bump manually by editing the values below, or via `BUMP=major|minor|patch pio run`.
#pragma once

#define FIRMWARE_VERSION_MAJOR {major}
#define FIRMWARE_VERSION_MINOR {minor}
#define FIRMWARE_VERSION_PATCH {patch}
#define FIRMWARE_VERSION_STRING "{major}.{minor}.{patch}"
"""

    os.makedirs(os.path.dirname(HEADER_PATH), exist_ok=True)
    with open(HEADER_PATH, "w") as f:
        f.write(content)


bump = os.environ.get("BUMP", "").strip().lower()

major, minor, patch = read_existing_version()

if bump in ("major", "minor", "patch"):
    major, minor, patch = apply_bump(major, minor, patch, bump)
    print(f"[generate_version] Bumped ({bump}) -> {major}.{minor}.{patch}")
elif not os.path.isfile(HEADER_PATH):
    print(f"[generate_version] Created firmware_version.hpp -> {major}.{minor}.{patch}")
else:
    print(f"[generate_version] Firmware version unchanged: {major}.{minor}.{patch}")

write_header(major, minor, patch)