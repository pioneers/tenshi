#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)
cd $SCRIPT_PATH

if diff /etc/avrdude.conf avrdude.conf >/dev/null ; then
  echo
else
  echo "$(tput setaf 1)Warning: Your /etc/avrdude.conf file may not be correct.
Press Ctrl+C and then run ./tools/setup_avrdude.sh to install the correct file
or press [Enter] to continue anyways.$(tput sgr 0)"
  read
fi

set -x #echo on
sudo avrdude -c usbtiny -p attiny1634 -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0x1F:m
sudo avrdude -c usbtiny -p attiny1634 -U flash:w:../build/artifacts/smartsensor_fw/opt/smartsensor_fw.hex:i
