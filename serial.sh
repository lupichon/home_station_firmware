#!/bin/bash
set -e

PORT="${1:-/dev/ttyUSB0}"

arduino-cli monitor -p "$PORT" -c baudrate=115200