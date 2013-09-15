#!/usr/bin/env python

# TODO(rqou): refactor to share code with the controller version

from __future__ import print_function
import os
import subprocess
import sys
import time

def get_git_hash():
    head_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip()
    if isinstance(head_hash, bytes):
        head_hash = head_hash.decode()
    return head_hash

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
        if not 'BUILD_NUMBER' in os.environ:
            ver[2] = '0'
        else:
            ver[2] = os.environ['BUILD_NUMBER']

    return '.'.join(ver)

def get_working_dir_clean():
    git_status = subprocess.check_output(['git', 'status', '--porcelain', '-uno']).strip()
    return len(git_status) == 0

def get_building_from_jenkins():
    if not 'JOB_NAME' in os.environ:
        return False
    if not os.environ['JOB_NAME'].endswith("-committed"):
        return False
    return True

def get_build_time():
    return time.strftime('%Y%m%d%H%M%S')

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

    githash = get_git_hash()
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

if __name__=='__main__':
    main()
