#!/bin/bash -xe

ARGS="$@"
if [ -z "$ARGS" ]; then
  ARGS='configure
        build_network/release_emscripten
        build_lua/release_emscripten
        build_vm/release_emscripten'
fi

./build.sh "$ARGS"
