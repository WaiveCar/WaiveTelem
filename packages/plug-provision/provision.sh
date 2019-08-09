#!/bin/bash

export TEMPLATE=./config/hyukia.txt
export DEFAULT_FIRMWARE=../plug-firmware/.pio/build/waive1000/firmware.bin
export SD_VOLUME=/Volumes/SDCARD
#export SD_VOLUME=/Volumes/E 

echo ""
echo "Connect Atmel-Ice to the device, and insert SD to the PC/MAC"

.  ../plug-firmware/detectDevice.sh

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

echo ""
echo "If there are no error messages, everything should be successful. Please insert the SD to the device."
