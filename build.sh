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

. tools/begin-build.sh

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

WAF_ARGS="$@"
if [ -z "$WAF_ARGS" ]; then
  WAF_ARGS="configure build"
fi

./waf $WAF_ARGS

./angel-player/build.sh

# Run unit test for Angel Player
# TODO(rqou): Fix the fact that this has to be run at the end of the build
# process and can't be run at the beginning.
./angel-player/run-tests.py
