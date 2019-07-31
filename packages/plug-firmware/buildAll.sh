#!/bin/bash

. setEnv.sh
pio run -v -e waive1000 && pio run -v -e mkrnb1500 && pio run -v -e mkr1000USB
