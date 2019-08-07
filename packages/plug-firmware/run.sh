#!/bin/bash

. setEnv.sh
pio run -t upload && sleep 3 && pio device monitor
