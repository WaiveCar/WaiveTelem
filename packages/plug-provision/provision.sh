#!/bin/bash

export CAR_TEMPLATE=./config/CAR_IONIQ.TXT
export MQTT_TEMPLATE=./config/MQTT_AWS.TXT
export DEFAULT_FIRMWARE=../plug-firmware/s3/waive1000_1.2.1_d9239cbbf7ddb4a233eeffce327bf0c98fe1464b3ae75433dcbe682b8ffd0437

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
