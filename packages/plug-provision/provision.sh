#!/bin/bash

export PLUG_ID=waive-2
export TEMPLATE=./config/hyukia.txt
export DEFAULT_FIRMWARE=../plug-firmware/s3/waive1000_1.0.5_8775e867506fdff30b443f50837b389b47ff8079bec349c0251a724ff20e0711
export SD_VOLUME=/Volumes/SDCARD

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
#. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

