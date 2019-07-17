#!/bin/bash

echo ""
echo "Running generateCSR.sh"

pio run -v --target upload
sleep 5

case "$(uname -s)" in
   Darwin)
      CSR=$(./getCSR.sh | head -1) # get first line of the serial port output
      CSR=${CSR%??} # remove last two characters
      ;;
   *)
      echo 'other OS'
      pio device monitor --quiet > csr.txt &
      PID=$!
      sleep 2
      kill $PID
      CSR=$(cat csr.txt)
      CSR=${CSR%???}
      rm -rf csr.txt
      ;;
esac

echo ${CSR}
echo ${CSR} > ./csr/${PLUG_ID}.csr

