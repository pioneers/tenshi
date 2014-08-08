#!/bin/bash -xe

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
VENDOR_JS=angel-player/src/chrome/content/vendor-js/
cp build/vm/release_emscripten/vm/angelic/src/ngl_vm.js $VENDOR_JS
cp build/lua/release_emscripten/vm/lua/lua.js $VENDOR_JS
cp build/network/release_emscripten/network/ndl3.js $VENDOR_JS

# Main build
for dir in angel-player
do
	./$dir/build.sh
done

# Run unit test for Angel Player
# TODO(rqou): Fix the fact that this has to be run at the end of the build
# process and can't be run at the beginning.
./angel-player/run-tests.py
