#!/bin/bash

PLUG_ID=plug-1
MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com

mkdir ./config/${PLUG_ID}
pio run -v --target upload
sleep 5

# CSR=$(./getCSR.sh | head -1) # get first line of the serial port output
# CSR=${CSR%??} # remove last two characters
pio device monitor --quiet > cert.txt &
PID=$!
sleep 3
kill $PID
CSR=$(cat cert.txt)
CSR=${CSR%???}
echo ${CSR}
rm -rf cert.txt

REG_OUTPUT=$(aws iot register-thing --template-body file://templateBody.json --parameters "{\"ThingName\": \"${PLUG_ID}\", \"CSR\": \"${CSR}\"}")
echo ${REG_OUTPUT}

echo ${REG_OUTPUT} | jq "{id: \"${PLUG_ID}\", mqttBrokerUrl: \"${MQTT_BROKER}\", mqttBrokerCert: .certificatePem, gps: {telemetry: true, inRideInterval: 30, notInRideInterval: 900}}" > ./config/${PLUG_ID}/config.txt
