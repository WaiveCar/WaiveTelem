#!/bin/bash

export DEVICE_ID=waive-1
export S3_HOST=waiveplug.s3.us-east-2.amazonaws.com

SHA=$(openssl dgst -sha256 -hmac "https failed" config/$DEVICE_ID/CONFIG.TXT)
SHA=${SHA/* }
FILENAME="config_${DEVICE_ID}_${SHA}"
cp config/$DEVICE_ID/CONFIG.TXT s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"CONFIG.TXT"}}}'

echo ""
aws s3 sync s3 s3://waiveplug
