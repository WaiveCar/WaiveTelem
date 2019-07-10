#!/bin/bash

CERT=$(cat cert/${PLUG_ID}.cert)
echo "{ \"id\": \"${PLUG_ID}\", \"mqttBrokerUrl\": \"${MQTT_BROKER}\", \"mqttBrokerCert\": ${CERT} }" | jq --from-file ${TEMPLATE} > ./config/${PLUG_ID}/config.txt
