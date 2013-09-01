#!/bin/bash -xe

# Set up tools (assumes jenkins copied them or manually copied into tools directory)
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:`pwd`/tools/arm-toolchain/bin

./waf configure build
