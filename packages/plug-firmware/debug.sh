#!/bin/bash

. setEnv.sh
export DEBUG='-D DEBUG'
pio run -t upload && sleep 2

mkdir -p nosave
~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump -d -S -j .text .pio/build/waive1000/firmware.elf > nosave/dis.txt

pio device monitor
