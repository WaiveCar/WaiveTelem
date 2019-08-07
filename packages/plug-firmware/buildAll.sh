#!/bin/bash

. setEnv.sh
pio run -e waive1000 && pio run -e mkrnb1500 && pio run -e mkr1000USB
