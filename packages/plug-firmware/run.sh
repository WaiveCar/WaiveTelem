#!/bin/bash

pio run -t upload && sleep 3 && pio device monitor
