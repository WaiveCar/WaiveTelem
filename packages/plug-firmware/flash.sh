#!/bin/bash

PORT=/dev/cu.usbmodem14101 # check with pio device list
stty -f ${PORT} 1200 \
  && sleep 3 \
  && stty stop /dev/cu.usbmodem14101 \
  && sleep 2; ~/.platformio/packages/tool-bossac/bossac --info --debug --port "${PORT}" --erase --write --verify --reset -U true .pioenvs/mkr1000USB/firmware.bin
