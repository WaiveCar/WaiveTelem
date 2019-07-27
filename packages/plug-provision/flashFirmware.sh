#!/bin/bash

echo ""
echo "Running flashFirmware.sh"

~/.platformio/packages/tool-openocd/bin/openocd -d2 -s ~/.platformio/packages/tool-openocd/scripts -f interface/cmsis-dap.cfg \
   -c "set CHIPNAME at91samd21g18" -f target/at91samdXX.cfg -c "program {${DEFAULT_FIRMWARE}} 0x2000 verify reset; shutdown;"

