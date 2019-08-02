#!/bin/bash

. setEnv.sh
. buildAll.sh

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/waive1000/firmware.bin)
SHA=${SHA/* }
FILENAME="waive1000_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/waive1000/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"UPDATE.BIN"}}}'

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkrnb1500/firmware.bin)
SHA=${SHA/* }
FILENAME="mkrnb1500_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/mkrnb1500/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"UPDATE.BIN"}}}'

echo ""
aws s3 sync s3 s3://waiveplug
