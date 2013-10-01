#!/bin/bash -xe

export PROJECT_ROOT_DIR=`pwd`

# Set up or download tools
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:$PROJECT_ROOT_DIR/tools/arm-toolchain/bin

mkdir -p build

# Main build
for dir in controller angel-player
do
	./$dir/build.sh
done

# Run cpplint
./tools/run-style-tool.py cpplint 2>&1 | tee build/cpplint.txt ; ( exit ${PIPESTATUS[0]} )
# Run pep8
./tools/run-style-tool.py pep8 2>&1 | tee build/pep8.txt ; ( exit ${PIPESTATUS[0]} )
# Run jshint
./tools/run-style-tool.py jshint 2>&1 | tee build/jshint.txt ; ( exit ${PIPESTATUS[0]} )
