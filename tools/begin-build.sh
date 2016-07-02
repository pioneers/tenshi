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
