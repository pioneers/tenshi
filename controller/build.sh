#!/bin/bash -xe

# Set up tools (assumes jenkins copied them or manually copied into tools directory)
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:`pwd`/tools/arm-toolchain/bin

# Main compile process
./waf configure build

# Run cpplint
./tools/run-cpplint.py 2>&1 | tee build/cpplint.txt ; ( exit ${PIPESTATUS[0]} )
