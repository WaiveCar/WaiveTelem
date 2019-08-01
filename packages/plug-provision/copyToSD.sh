#!/bin/bash

echo ""
echo "Running copyToSD.sh"

cp -v $DEFAULT_FIRMWARE $SD_VOLUME/DEFAULT.BIN
cp -v ./config/$DEVICE_ID/CONFIG.TXT $SD_VOLUME/DEFAULT.CFG
cp -v ./config/$DEVICE_ID/* $SD_VOLUME/CONFIG.TXT
