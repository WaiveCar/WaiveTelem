#!/bin/bash

export CAR_TEMPLATE=./config/CAR_IONIQ.TXT
export MQTT_TEMPLATE=./config/MQTT_AWS.TXT
export DEFAULT_FIRMWARE=../plug-firmware/s3/waive1000_1.2.9_f6116d1f86bc4b68fc5dc7c6c9f1747d588b190f7f06ab2b5f3e4460e71566ba

echo ""
echo "Connect Atmel-Ice to the device, and insert SD to the PC/MAC"

. generateCSR.sh
. registerAWS.sh
. generateConfig.sh
#. formatSD.sh
. copyToSD.sh
. flashFirmware.sh

echo ""
echo "If there are no error messages, everything should be successful. Please insert the SD to the device."
