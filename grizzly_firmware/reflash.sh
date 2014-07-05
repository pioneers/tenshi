#!/bin/sh

echo "Connect Grizzly to test jig and AVR programmer."
echo

sudo avrdude -c usbtiny -p atmega32u4 -U flash:w:../build/artifacts/grizzly_firmware/opt/grizzly_firmware.hex:i

