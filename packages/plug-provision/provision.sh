#!/bin/bash

export PLUG_ID=waive-1
export TEMPLATE=./config/hyukia.txt
export DEFAULT_FIRMWARE=../plug-firmware/bin/1500_1.0.5_0703f7ea638442e31e9e6c4ae10726839eaaf391c49915d16618cfcc995682ac
export SD_VOLUME=/Volumes/SDCARD
#export SD_VOLUME=/d
#export BOARD=mkr1000USB
export BOARD=mkrnb1500

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
#. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

