#!/bin/bash

rm -rf ~/.platformio/boards
ln -s -f $PWD/bsp/boards ~/.platformio
rm -rf ~/.platformio/packages/framework-arduinosam/variants/waive1000
ln -s -f $PWD/bsp/waive1000 ~/.platformio/packages/framework-arduinosam/variants
sed -i'.bak' -e 's/adapter_khz\ 400/adapter_khz\ 5000/g' ~/.platformio/packages/tool-openocd/scripts/target/at91samdXX.cfg
sed -i'.bak' -e 's/static\ volatile\ uint32_t\ _ulTickCount/volatile\ uint32_t\ _ulTickCount/g' ~/.platformio/packages/framework-arduinosam/cores/samd/delay.c

pio lib update

export FIRMWARE_VERSION=1.1.7

export S3_HOST=waiveplug.s3.us-east-2.amazonaws.com
export DEBUG=

