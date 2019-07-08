#!/bin/bash

pio run -v -t upload && sleep 3 && pio device monitor
