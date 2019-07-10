#!/bin/bash

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

