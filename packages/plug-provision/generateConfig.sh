#!/bin/bash

echo ""
echo "Running generateConfig.sh"

MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com
CERT=$(cat cert/${PLUG_ID}.cert)

mkdir ./config/${PLUG_ID}
echo "{ \"id\": \"${PLUG_ID}\", \"mqttBrokerUrl\": \"${MQTT_BROKER}\", \"mqttBrokerCert\": ${CERT} }" | jq --from-file ${TEMPLATE} > ./config/${PLUG_ID}/CONFIG.TXT
