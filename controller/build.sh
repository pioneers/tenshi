#!/bin/bash -xe

CONTROLLER_MAIN_DIR=$PROJECT_ROOT_DIR/controller

pushd $CONTROLLER_MAIN_DIR

# Main compile process
./waf configure build

popd
