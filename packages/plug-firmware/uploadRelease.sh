#!/bin/bash

. buildAll.sh

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/waive1000/firmware.bin)
SHA=${SHA/* }
FILENAME="waive1000_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/waive1000/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"ETADPU.BIN"}}}'
echo ""
aws s3 cp s3/$FILENAME s3://waiveplug
aws s3api put-object-acl --bucket waiveplug --key $FILENAME --acl public-read
cp .pio/build/waive1000/firmware.elf disassembly/$FILENAME.elf
~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump -d -S -j .text .pio/build/waive1000/firmware.elf > disassembly/$FILENAME.txt

SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkrnb1500/firmware.bin)
SHA=${SHA/* }
FILENAME="mkrnb1500_${FIRMWARE_VERSION}_${SHA}"
cp .pio/build/mkrnb1500/firmware.bin s3/$FILENAME
echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"ETADPU.BIN"}}}'
echo ""
aws s3 cp s3/$FILENAME s3://waiveplug
aws s3api put-object-acl --bucket waiveplug --key $FILENAME --acl public-read
cp .pio/build/waive1000/firmware.elf disassembly/$FILENAME.elf
~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump -d -S -j .text .pio/build/mkrnb1500/firmware.elf > disassembly/$FILENAME.txt

# SHA=$(openssl dgst -sha256 -hmac "https failed" .pio/build/mkr1000USB/firmware.bin)
# SHA=${SHA/* }
# FILENAME="mkr1000USB_${FIRMWARE_VERSION}_${SHA}"
# cp .pio/build/mkr1000USB/firmware.bin s3/$FILENAME
# echo '{"desired":{"download":{"host":"'"${S3_HOST}"'", "from":"'"${FILENAME}"'", "to":"ETADPU.BIN"}}}'

