#!/bin/sh
sudo avrdude -c usbtiny -p atmega32u4 -U lfuse:w:0xff:m -U hfuse:w:0xD9:m -U efuse:w:0xFB:m
sudo avrdude -c usbtiny -p atmega32u4 -U flash:w:build/programs/firmware.hex:i

