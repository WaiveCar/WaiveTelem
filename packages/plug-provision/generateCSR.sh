#!/bin/bash

echo ""
echo "Running generateCSR.sh"

pio run -v --target upload
sleep 5

case "$(uname -s)" in
   Darwin)
      JSON=$(./getCSR.sh | head -1) # get first line of the serial port output
      ;;
   *)
      echo 'other OS'
      pio device monitor --quiet > csr.txt &
      PID=$!
      sleep 15
      kill $PID
      JSON=$(cat csr.txt)
      rm -rf csr.txt
      ;;
esac

echo $JSON
export DEVICE_ID=$(echo $JSON | jq --raw-output ".serial")
echo $JSON | jq ".csr" | sed -e 's/^"//' -e 's/"$//' > ./csr/${DEVICE_ID}.csr

