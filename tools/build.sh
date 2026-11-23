#!/usr/bin/env bash
# Build (and optionally flash) a sketch from the command line with arduino-cli.
#
#   tools/build.sh                                  # compile the totem
#   tools/build.sh totem /dev/cu.usbmodem1101       # compile, then upload
#   tools/build.sh totem_panel_test /dev/cu.usbmodem1101
#   tools/build.sh find_whoop_mac /dev/cu.usbmodem1101
#
# Find the port with `arduino-cli board list`. The FQBN below is the same as
# the Arduino IDE's Tools menu settings in the build guide; keep them in sync.
#
# One-time setup:
#   arduino-cli core install esp32:esp32
#   arduino-cli lib install FastLED NimBLE-Arduino@2.5.1
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SKETCH="${1:-totem}"
PORT="${2:-}"
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB"
DIR="$REPO/firmware/$SKETCH"

if [ ! -f "$DIR/$SKETCH.ino" ]; then
  echo "error: no sketch at firmware/$SKETCH/$SKETCH.ino" >&2
  exit 1
fi

if [ "$SKETCH" = "totem" ] && [ ! -r "$DIR/secrets.h" ]; then
  echo "error: firmware/totem/secrets.h is missing." >&2
  echo "       cp firmware/totem/secrets.h.example firmware/totem/secrets.h, then fill in your strap's MAC." >&2
  exit 1
fi

if [ -n "$PORT" ]; then
  arduino-cli compile --upload -b "$FQBN" -p "$PORT" "$DIR"
  echo
  echo "Flashed. Watch it with: python3 tools/serial_monitor.py $PORT"
else
  arduino-cli compile -b "$FQBN" "$DIR"
fi
