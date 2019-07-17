#!/bin/bash

PLUG_ID=plug-1
TEMPLATE=./config/hyukia.txt
DEFAULT_FIRMWARE=../plug-firmware/bin/1000_1.0.4_9ccd5d8eb348015145a4df52cd29e7e233768a9d2fc475e4aab4773fc5cccb66
#SD_VOLUME=/Volumes/SDCARD
SD_VOLUME=/d
#BOARD=mkrnb1000
BOARD=mkrnb1500

#. generateCSR.sh
#. registerAWS.sh
. generateConfig.sh
. flashFirmware.sh
#. formatSD.sh
. copyToSD.sh

