#!/bin/bash

. setEnv.sh
. buildAll.sh

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkr1000USB/firmware.bin)
SHA=${SHA/* }
FILENAME="1000_${FIRMWARE_VERSION}_${SHA}"
cp -v .pio/build/mkr1000USB/firmware.bin bin/$FILENAME
echo ""
echo '{"desired": {"download": {"host": "waiveplug.s3.us-east-2.amazonaws.com", "from": "'"${FILENAME}"'", "to": "UPDATE.BIN"}}}'
echo ""

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkrnb1500/firmware.bin)
SHA=${SHA/* }
FILENAME="1500_${FIRMWARE_VERSION}_${SHA}"
cp -v .pio/build/mkrnb1500/firmware.bin bin/$FILENAME
echo ""
echo '{"desired": {"download": {"host": "waiveplug.s3.us-east-2.amazonaws.com", "from": "'"${FILENAME}"'", "to": "UPDATE.BIN"}}}'
echo ""

aws s3 sync bin s3://waiveplug
