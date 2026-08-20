#!/bin/bash
set -e

arduino-cli compile \
  --fqbn esp32:esp32:esp32doit-devkit-v1 \
  --build-property build.partitions=partitions \
  --build-property upload.maximum_size=3145728 \
  .