#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)
cd $SCRIPT_PATH

set -x #echo on
sudo avrdude -c usbtiny -p attiny1634 -U flash:w:../build/artifacts/smartsensor_fw/opt/smartsensor_fw.hex:i
