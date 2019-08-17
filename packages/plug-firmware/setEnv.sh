#!/bin/bash

ln -s -f $PWD/bsp/boards ~/.platformio
ln -s -f $PWD/bsp/waive1000 ~/.platformio/packages/framework-arduinosam/variants
sed -i'.bak' -e 's/adapter_khz\ 400/adapter_khz\ 5000/g' ~/.platformio/packages/tool-openocd/scripts/target/at91samdXX.cfg
sed -i'.bak' -e 's/static\ volatile\ uint32_t\ _ulTickCount/volatile\ uint32_t\ _ulTickCount/g' ~/.platformio/packages/framework-arduinosam/cores/samd/delay.c

export FIRMWARE_VERSION=1.0.8

export S3_HOST=waiveplug.s3.us-east-2.amazonaws.com
export DEBUG=

