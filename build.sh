#!/bin/bash -xe

export PROJECT_ROOT_DIR=`pwd`

# Set up or download tools
./tools/extract-tools.sh

# TODO(rqou): Less hacky
export PATH=$PATH:$PROJECT_ROOT_DIR/tools/arm-toolchain/bin

mkdir -p build

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
cp build/vm/release_emscripten/controller/vm/src/ngl_vm.js angel-player/src/chrome/content/vendor-js/
# TODO(rqou): Really fix this hack
sed -i 's#typeof process === '"'"'object'"'"' && ##g' angel-player/src/chrome/content/vendor-js/ngl_vm.js
sed -i 's#require('"'"'fs'"'"')#require('"'"'jetpack/sdk/io/fs'"'"')#g' angel-player/src/chrome/content/vendor-js/ngl_vm.js
sed -i 's#require('"'"'path'"'"')#require('"'"'jetpack/sdk/fs/path'"'"')#g' angel-player/src/chrome/content/vendor-js/ngl_vm.js
sed -i 's#Module\['"'"'arguments'"'"'\] = process\['"'"'argv'"'"'\].slice(2);##g' angel-player/src/chrome/content/vendor-js/ngl_vm.js
sed -i 's#var fs = require("fs");#var fs = require("jetpack/sdk/io/fs");#g' angel-player/src/chrome/content/vendor-js/ngl_vm.js
# TODO(rqou): This is really bad
sed -i 's#!!process.platform.match(/^win/);#false;#g' angel-player/src/chrome/content/vendor-js/ngl_vm.js

# Main build
for dir in angel-player
do
	./$dir/build.sh
done
