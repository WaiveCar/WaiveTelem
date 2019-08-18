#!/bin/bash

. setEnv.sh
export DEBUG='-D DEBUG'
pio run -t upload && sleep 2 && pio device monitor
