#!/bin/bash

echo "Running flashFirmware.sh"

case "$(uname -s)" in
   Darwin)
      PORT=$(pio device list --json-output | jq '.[] | select(.description | contains("Arduino")) | .port ')
      PORT="${PORT%\"}"
      PORT="${PORT#\"}"

      stty -f ${PORT} 1200 \
        && sleep 3 \
        && ~/.platformio/packages/tool-bossac/bossac --info --debug --port "${PORT}" --erase --write --verify --reset -U true ${DEFAULT_FIRMWARE}

      ;;
   *)
      (cd ../plug-firmware; pio run -e $BOARD -t build)
      ;;
esac

