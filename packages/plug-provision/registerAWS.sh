#!/bin/bash

echo ""
echo "Running registerAWS.sh"

CSR=$(cat csr/${DEVICE_ID}.csr)
REG_OUTPUT=$(aws iot register-thing --template-body file://templateBody.json --parameters "{\"ThingName\": \"${DEVICE_ID}\", \"CSR\": \"${CSR}\"}")
echo ${REG_OUTPUT}
echo ${REG_OUTPUT} | jq ".certificatePem" > ./cert/${DEVICE_ID}.cert
