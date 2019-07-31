#!/bin/bash

. setEnv.sh
pio run -v -t upload && sleep 5 && pio device monitor
