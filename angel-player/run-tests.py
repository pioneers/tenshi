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
import os
import re
import subprocess
import shutil
import sys
sys.path.insert(0, os.path.join(sys.path[0], '../tools'))
from eagle_util_funcs import start_xvfb, kill_xvfb

def prep_testing_dir(proj_root):
    # Remove and recreate the test working directory
    testing_working_dir = os.path.join(proj_root, 'build/angel-player-test')
    shutil.rmtree(testing_working_dir, True)
    os.mkdir(testing_working_dir)

    def symlink_from_src_to_test(fn):
        os.symlink(
            os.path.join(proj_root, 'angel-player/src/' + fn),
            os.path.join(proj_root, 'build/angel-player-test/' + fn))

    def copy_from_src_to_test(fn):
        shutil.copyfile(
            os.path.join(proj_root, 'angel-player/src/' + fn),
            os.path.join(proj_root, 'build/angel-player-test/' + fn))

    # Symlink things
    symlink_from_src_to_test('chrome.manifest')
    symlink_from_src_to_test('chrome')
    
    # Copy application.ini (XULRunner is clever and reads through symlinks)
    copy_from_src_to_test('application.ini')

    # Copy preferences (don't symlink, because we will mutate them)
    shutil.copytree('angel-player/src/defaults',
                    'build/angel-player-test/defaults')

    # Now we mutate prefs.js to change toolkit.defaultChromeURI
    f = open('build/angel-player-test/defaults/preferences/prefs.js', 'r')
    old_prefs = f.read()
    f.close()
    new_prefs = re.sub(
        'pref\([\'"]toolkit.defaultChromeURI[\'"], [\'"].*[\'"]\);',
        'pref("toolkit.defaultChromeURI", '
        '"chrome://angel-player/content/main-tests.xul");',
        old_prefs)
    f = open('build/angel-player-test/defaults/preferences/prefs.js', 'w')
    f.write(new_prefs)
    f.close()

    # Remove and recreate the test profile directory
    testing_profile_dir = os.path.join(proj_root, 'build/angel-player-profile')
    shutil.rmtree(testing_profile_dir, True)
    os.mkdir(testing_profile_dir)
    open(os.path.join(testing_profile_dir, 'profile.ini'), 'w').close()

    return (testing_working_dir, testing_profile_dir)


def main():
    # We're in the $proj-root/angel-player directory, get $proj-root directory
    proj_root = os.path.abspath(os.path.join(sys.path[0], '..'))

    testing_working_dir, testing_profile_dir = prep_testing_dir(proj_root)

    xulrunner_path = os.path.abspath(os.path.join(
        proj_root,
        'build/angel-player/angel-player.app/xul-lin64/xulrunner'))
    print(xulrunner_path)

    # Start xvfb
    xvfb, display_num = start_xvfb()

    xulrunner_p = subprocess.Popen([
        xulrunner_path,
        os.path.join(testing_working_dir, 'application.ini'),
        '-profile',
        testing_profile_dir,
    ], stderr = subprocess.STDOUT, stdout = subprocess.PIPE)
    log_results = xulrunner_p.communicate()[0]

    kill_xvfb(xvfb)

    # Print out log results
    print("*" * 80)
    print(log_results)
    print("*" * 80)

    tests_passed = "All tests passed!" in log_results

    if tests_passed:
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
