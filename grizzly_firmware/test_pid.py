#!/usr/bin/env python

# Licensed to Pioneers in Engineering under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  Pioneers in Engineering licenses
# this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License

from __future__ import print_function

import serial
import time
import atexit

filename = '/dev/ttyUSB0'

dev = serial.Serial(filename, 115200, timeout=0)

@atexit.register
def stop_motor():
    print('Ending test.')
    dev.write(b'[ 0x1e 1 5 0 0 0 0 0 0 0 ]\n')
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

dev.write(b'[ 0x1e 1 5 0 0 0 0 100 0 0 ]\n')
# Move motor forward at full speed

print('Moving forward at full speed.')

time.sleep(3)

dev.write(b'[ 0x1e 1 5 0 0 0 0 0x9c 0xff 0 ]\n')
# Move

print('Moving backward at full speed.')

time.sleep(3)

dev.write(b'[ 0x1e 1 5 0 0 0 0 0 0 0 ]\n')
# Stop motor
