#!/usr/bin/env python
from __future__ import print_function

import os
import os.path
import subprocess
import sys
import fnmatch


def find_files_in_directory(dirname, exclusion_patterns):
    found_files = []
    for root, dirs, files in os.walk('.'):
        for filename in files:
            filepath = os.path.join(root, filename)
            # Hack
            if filepath.startswith("./"):
                filepath = filepath[2:]
            # Check whether we want this file or not
            included = False
            for pat in dirname:
                if fnmatch.fnmatch(filepath, pat):
                    included = True
                    break
            if not included:
                continue

            # Check whether we explicitly excluded it
            excluded = False
            for pat in exclusion_patterns:
                if fnmatch.fnmatch(filepath, pat):
                    excluded = True
                    break
            if not excluded:
                found_files.append(filepath)
    return found_files


def read_exclusion_patterns(toolname):
    try:
        f = open(os.path.dirname(sys.argv[0]) +
                 "/" + toolname + "-exclusions.txt", "r")
    except IOError:
        # No such file, so assume no rules
        return []

    patterns = []
    l = f.readline()
    while l:
        if not l.startswith('#'):
            patterns.append(l.strip())
        l = f.readline()
    return patterns


def read_source_dirs(toolname):
    f = open(os.path.dirname(sys.argv[0]) + "/" + toolname + "-files.txt", "r")

    lines = []
    l = f.readline()
    while l:
        if not l.startswith("#"):
            lines.append(l.strip())
        l = f.readline()
    return lines


def main():
    if len(sys.argv) < 2:
        print("Usage: %s cpplint|pep8|jshint" % sys.argv[0])
        sys.exit(1)

    styletool = sys.argv[1]

    exclusion_patterns = read_exclusion_patterns(styletool)
    source_dirs = read_source_dirs(styletool)
    source_files = find_files_in_directory(source_dirs, exclusion_patterns)

    if styletool == "cpplint":
        exit_code = subprocess.call([
            './tools/cpplint.py',
            '--filter=-legal/copyright,-readability/casting',
            '--extensions=c,cpp,h,template'
            ] + source_files)
        sys.exit(exit_code)
    elif styletool == "pep8":
        exit_code = subprocess.call([
            'pep8',
            ] + source_files)
        sys.exit(exit_code)
    elif styletool == "jshint":
        # Hack for Jenkins violations being stupid
        source_files = list(map(lambda x: os.getcwd() + "/" + x,
                                source_files))
        exit_code = subprocess.call([
            'jshint',
            '-c', os.path.dirname(sys.argv[0]) + "/" + "jshintrc",
            '--reporter=jslint',
            ] + source_files)
        sys.exit(exit_code)
    elif styletool == "csslint":
        exit_code = subprocess.call([
            'csslint',
            '--format=lint-xml',
            ] + source_files)
        sys.exit(exit_code)
    else:
        print("Unknown style tool %s" % styletool)
        sys.exit(1)

if __name__ == '__main__':
    main()
