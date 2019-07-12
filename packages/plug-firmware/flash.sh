#!/bin/bash

PORT=$(pio device list --json-output | jq '.[] | select(.description | contains("Arduino")) | .port ')
PORT="${PORT%\"}"
PORT="${PORT#\"}"

stty -f ${PORT} 1200 \
  && sleep 3 \
  && ~/.platformio/packages/tool-bossac/bossac --info --debug --port "${PORT}" --erase --write --verify --reset -U true bin/mkr1000USB/010003.bin
