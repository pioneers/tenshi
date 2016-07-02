#!/usr/bin/env python

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

from __future__ import print_function

import subprocess
import sys
import os

node = subprocess.Popen(['node', '--harmony',
                         os.path.join(sys.argv[2],
                                      'correct_contents',
                                      'correct_contents.js'),
                         sys.argv[1]])

node.communicate()

with open('correct_contents.bin', 'rb') as f:
    buf = bytearray(f.read())
    assert len(buf) == 1024
    for index, byte in enumerate(buf):
        assert index & 0xff == byte

sys.exit(node.returncode)
