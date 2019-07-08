#!/bin/bash

TEMPLATE=./config/hyukia.txt
PLUG_ID=plug-1
MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com

mkdir ./config/${PLUG_ID}
pio run -v --target upload
sleep 3

case "$(uname -s)" in

   Darwin)
      CSR=$(./getCSR.sh | head -1) # get first line of the serial port output
      CSR=${CSR%??} # remove last two characters
      ;;

  #  Linux)
  #    echo 'Linux'
  #    ;;

  #  CYGWIN*|MINGW32*|MSYS*)
  #    echo 'MS Windows'
  #    ;;

   *)
      echo 'other OS'
      pio device monitor --quiet > cert.txt &
      PID=$!
      sleep 2
      kill $PID
      CSR=$(cat cert.txt)
      CSR=${CSR%???}
      echo ${CSR}
      rm -rf cert.txt
      ;;
esac

REG_OUTPUT=$(aws iot register-thing --template-body file://templateBody.json --parameters "{\"ThingName\": \"${PLUG_ID}\", \"CSR\": \"${CSR}\"}")
echo ${REG_OUTPUT}

echo "{ \"id\": \"${PLUG_ID}\", \"mqttBrokerUrl\": \"${MQTT_BROKER}\", \"regOutput\": ${REG_OUTPUT} }" | jq --from-file ${TEMPLATE} > ./config/${PLUG_ID}/config.txt