#!/bin/bash

export PLUG_ID=plug-1
export TEMPLATE=./config/hyukia.txt
export DEFAULT_FIRMWARE=../plug-firmware/bin/1000_1.0.4_9ccd5d8eb348015145a4df52cd29e7e233768a9d2fc475e4aab4773fc5cccb66
export SD_VOLUME=/Volumes/SDCARD
#export SD_VOLUME=/d
#export BOARD=mkr1000USB
export BOARD=mkrnb1500

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

