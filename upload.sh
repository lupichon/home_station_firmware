#!/bin/bash
set -e

PORT="${1:-/dev/ttyUSB0}"

arduino-cli upload \
  -p "$PORT" \
  --fqbn esp32:esp32:esp32doit-devkit-v1 \
  .