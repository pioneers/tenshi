#!/bin/bash -xe

# Set up tools (assumes jenkins copied them or manually copied into tools directory)
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:`pwd`/tools/arm-toolchain/bin

# Set up waf
if [ ! -e waf ]
then
	wget -O waf http://waf.googlecode.com/files/waf-1.7.11
	chmod +x waf
fi

./waf configure build
