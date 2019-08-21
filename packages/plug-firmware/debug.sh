#!/bin/bash

. setEnv.sh
export DEBUG='-D DEBUG'
pio run -t upload && sleep 3 && pio device monitor
