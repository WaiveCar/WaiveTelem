#!/bin/bash

. setEnv.sh
. buildAll.sh

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkr1000USB/firmware.bin)
SHA=${SHA/* }
FILENAME="1000_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/mkr1000USB/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"UPDATE.BIN"}}}'

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkrnb1500/firmware.bin)
SHA=${SHA/* }
FILENAME="1500_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/mkrnb1500/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"UPDATE.BIN"}}}'

echo ""
aws s3 sync s3 s3://waiveplug
