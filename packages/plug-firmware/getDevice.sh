#!/bin/bash

DEVICE=$(pio device list --json-output | jq '.[] | select(.description | contains("00")) | .description ')
echo "Connected device is $DEVICE"
if [ "$DEVICE" == '"Arduino MKR1000"' ]
then
  export DEFAULT_BOARD=mkr1000USB
elif [ "$DEVICE" == '"Arduino MKR NB 1500"' ]
then
  export DEFAULT_BOARD=mkrnb1500
else
  export DEFAULT_BOARD=waive1000
fi

#export DEFAULT_BOARD=mkr1000USB