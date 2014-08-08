#!/bin/bash

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
