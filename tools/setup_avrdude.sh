#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)
cd $SCRIPT_PATH

set -x #echo on
sudo cp avrdude.conf /etc/