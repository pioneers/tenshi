#!/usr/bin/env python
from __future__ import print_function

import serial
import time
import atexit

filename = '/dev/ttyUSB0'

dev = serial.Serial(filename, 115200, timeout=0)

@atexit.register
def stop_motor():
    print('Ending test.')
    dev.write(b'[ 0x1e 1 3 0 0 0 0 0 0 0 ]\n')
    time.sleep(0.5)

dev.write(b'm\n')
# Switch mode

dev.write(b'4\n')
# I2C

dev.write(b'3\n')
# 100KHz

time.sleep(0.5)

dev.write(b'P\n')
# Enable pull-ups

time.sleep(0.5)

dev.write(b'[ 0x1e 1 3 0 0 0 0 100 0 0 ]\n')
# Move motor forward at full speed

print('Moving forward at full speed.')

time.sleep(3)

dev.write(b'[ 0x1e 1 3 0 0 0 0 0x9c 0xff 0 ]\n')
# Move

print('Moving backward at full speed.')

time.sleep(3)

dev.write(b'[ 0x1e 1 3 0 0 0 0 0 0 0 ]\n')
# Stop motor
