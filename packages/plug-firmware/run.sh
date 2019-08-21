#!/bin/bash

. setEnv.sh
pio run -t upload && sleep 2 && pio device monitor
