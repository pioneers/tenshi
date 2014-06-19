#!/bin/bash -xe

export PROJECT_ROOT_DIR=`pwd`

# Set up or download tools
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:$PROJECT_ROOT_DIR/tools/arm-toolchain/bin:$PROJECT_ROOT_DIR/tools/emscripten-bin:$PROJECT_ROOT_DIR/tools/emscripten-bin/llvm/bin

if [[ -z "LLVM" ]] ; then
	export LLVM_DIR="$PROJECT_ROOT_DIR/emscripten-bin/llvm"
	export LLVM="$LLVM_DIR/bin"
	export PATH="$PATH:$PROJECT_ROOT_DIR/emscripten-bin:$LLVM"
fi

mkdir -p build/artifacts

# Run linters
linter_status=0
for tool in cpplint pep8 jshint csslint
do
  ./tools/run-style-tool.py $tool 2>&1 | tee build/${tool}.txt
  linter_status=$[${linter_status} || ${PIPESTATUS[0]}]
done

if [ $linter_status != 0 ] ; then
  exit $linter_status
fi

./waf configure build

# TODO(kzentner): Fix this hack?
cp build/vm/release_emscripten/vm/angelic/src/ngl_vm.js angel-player/src/chrome/content/vendor-js/
cp build/lua/release_emscripten/vm/lua/lua.js angel-player/src/chrome/content/vendor-js/

# Main build
for dir in angel-player
do
	./$dir/build.sh
done

# Run unit test for Angel Player
# TODO(rqou): Fix the fact that this has to be run at the end of the build
# process and can't be run at the beginning.
./angel-player/run-tests.py
