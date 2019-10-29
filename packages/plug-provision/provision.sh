#!/bin/bash

export CAR_TEMPLATE=./config/CAR_IONIQ.TXT
export MQTT_TEMPLATE=./config/MQTT_AWS.TXT
export DEFAULT_FIRMWARE=../plug-firmware/s3/waive1000_1.3.3_8b1d3c1d08b4706b0f2a251bebb4fd7774ab57278659ca6278b794a414f700b7

echo ""
echo "Connect to both SWD (w/ Atmel ICE) and USB of the device. Insert SD to the PC/MAC"

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
#. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

echo ""
echo "If there are no error messages, everything should be successful. Please insert the SD to the device."
