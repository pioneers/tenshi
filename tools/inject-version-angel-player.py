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

# TODO(rqou): refactor to share code with the controller version

from __future__ import print_function
import os
import subprocess
import sys
import time
from version_info_helpers import *


def get_version_info(orig_file):
    with open(orig_file, "r") as f:
        l = f.readline()
        while l:
            if l.startswith("Version="):
                ver = l[len("Version="):].strip().split('.')
                break
            l = f.readline()
    if len(ver) != 4:
        raise Exception("Malformed version line in input")

    # Special handling for build number
    if ver[2] == 'X':
        if 'BUILD_NUMBER' not in os.environ:
            ver[2] = '0'
        else:
            ver[2] = os.environ['BUILD_NUMBER']

    return '.'.join(ver)


def do_file_substitutions(version, build_id, infilename, outfilenames):
    replaced_contents = ""
    with open(infilename, "r") as inf:
        l = inf.readline()
        while l:
            if l.startswith("Version="):
                l = "Version=%s\n" % version
            if l.startswith("BuildID="):
                l = "BuildID=%s\n" % build_id

            replaced_contents = replaced_contents + l
            l = inf.readline()

    for outfilename in outfilenames:
        with open(outfilename, "w") as outf:
            outf.write(replaced_contents)


def main():
    if len(sys.argv) < 3:
        print("Usage: %s infile outfile(s)", sys.argv[0])
        sys.exit(1)

    githash = get_git_hash(False)
    version = get_version_info(sys.argv[1])
    working_dir_clean = get_working_dir_clean()
    building_from_jenkins = get_building_from_jenkins()
    build_time = get_build_time()

    version = version + "-0git" + githash
    if not working_dir_clean:
        version = version + "-dirty"
    if building_from_jenkins:
        version = version + "-jenkins"

    do_file_substitutions(version, build_time, sys.argv[1], sys.argv[2:])

if __name__ == '__main__':
    main()
