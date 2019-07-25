#!/bin/bash

export FIRMWARE_VERSION=1.0.5

export S3_HOST=waiveplug.s3.us-east-2.amazonaws.com

export WIFI_SSID=WaiveCar
export WIFI_PASSWORD=Waivecar1547

ln -s -f $PWD/boards ~/.platformio
ln -s -f $PWD/waive1000 ~/.platformio/packages/framework-arduinosam/variants

