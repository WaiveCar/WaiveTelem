#!/bin/bash

# PORT=/dev/cu.usbmodem14101 # check with pio device list
PORT=$(pio device list --json-output | jq '.[] | select(.description | contains("Arduino")) | .port ')
PORT="${PORT%\"}"
PORT="${PORT#\"}"

stty -f ${PORT} 1200 \
  && sleep 3 \
  && ~/.platformio/packages/tool-bossac/bossac --info --debug --port "${PORT}" --reset
