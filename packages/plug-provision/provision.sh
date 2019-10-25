#!/bin/bash

export CAR_TEMPLATE=./config/CAR_IONIQ.TXT
export MQTT_TEMPLATE=./config/MQTT_AWS.TXT
export DEFAULT_FIRMWARE=../plug-firmware/s3/waive1000_1.3.2_fc277bc970ec26a53f33b8c8cf6874598d1fdbbc73f2a1d3cc0b69f3920ba23e

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
