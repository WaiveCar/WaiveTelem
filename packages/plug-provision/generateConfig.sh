#!/bin/bash

echo ""
echo "Running generateConfig.sh"

MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com
CERT=$(cat cert/${DEVICE_ID}.cert)

mkdir ./config/${DEVICE_ID}
echo "{ \"id\": \"${DEVICE_ID}\", \"mqttBrokerUrl\": \"${MQTT_BROKER}\", \"mqttBrokerCert\": ${CERT} }" | jq --from-file ${MQTT_TEMPLATE} > ./config/${DEVICE_ID}/MQTT.TXT
cp ${CAR_TEMPLATE} ./config/${DEVICE_ID}/CONFIG.TXT