#!/bin/bash

#PLUG_ID=plug-1
#MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com
#SD_PATH=/Volumes/TELEMCONFIG/

# build
pio run --target upload
sleep 3
CSR=$(./getCSR.sh | head -1) # get first line of the serial port output
CSR=${CSR%??} # remove last two characters
echo ${CSR}

REG_OUTPUT=$(aws iot register-thing --template-body file://templateBody.json --parameters "{\"ThingName\": \"${PLUG_ID}\", \"CSR\": \"${CSR}\"}")
echo ${REG_OUTPUT}

echo ${REG_OUTPUT} | jq "{id: \"${PLUG_ID}\", mqttBrokerUrl: \"${MQTT_BROKER}\", mqttBrokerCert: .certificatePem, gps: {telemetry: true, inRideInterval: 30, notInRideInterval: 900}}" > ${SD_PATH}config.txt
