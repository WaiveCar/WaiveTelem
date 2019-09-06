#!/bin/bash

echo ""
echo "Running copyToSD.sh"

case "$(uname -s)" in
   Darwin)
      export SD_VOLUME=/Volumes/SDCARD
      ;;
   *)
      export SD_VOLUME=/E
      ;;
esac

cp -v $DEFAULT_FIRMWARE $SD_VOLUME/DEFAULT.BIN
cp -v ./config/$DEVICE_ID/CONFIG.TXT $SD_VOLUME/DEFAULT.CFG
cp -v ./config/$DEVICE_ID/* $SD_VOLUME
