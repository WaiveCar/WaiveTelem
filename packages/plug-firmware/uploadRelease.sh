#!/bin/bash

. setEnv.sh
. buildAll.sh

OUTPUT=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkr1000USB/firmware.bin)
OUTPUT=${OUTPUT/* }
FILENAME="1000_${FIRMWARE_VERSION}_${OUTPUT}"
cp .pio/build/mkr1000USB/firmware.bin bin/$FILENAME

OUTPUT=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkrnb1500/firmware.bin)
OUTPUT=${OUTPUT/* }
FILENAME="1500_${FIRMWARE_VERSION}_${OUTPUT}"
cp .pio/build/mkrnb1500/firmware.bin bin/$FILENAME

aws s3 sync bin s3://waiveplug