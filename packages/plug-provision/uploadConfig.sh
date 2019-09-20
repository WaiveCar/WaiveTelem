#!/bin/bash

export DEVICE_ID=0123CCBCCC98B697EE
export S3_HOST=waiveplug.s3.us-east-2.amazonaws.com

SHA=$(openssl dgst -sha256 -hmac "https failed" config/$DEVICE_ID/CONFIG.TXT)
SHA=${SHA/* }
FILENAME="config_${DEVICE_ID}_${SHA}"
cp config/$DEVICE_ID/CONFIG.TXT s3/$FILENAME
aws s3 sync s3 s3://waiveplug
aws s3api put-object-acl --bucket waiveplug --key $FILENAME --acl public-read
echo ""
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"CONFIG.TXT"}}}'

