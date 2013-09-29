#!/usr/bin/env python

import os
import os.path
import re
import subprocess
import sys

def find_files_in_directory(dirname, exclusion_patterns):
    found_files = []
    for root, dirs, files in os.walk(dirname):
        for filename in files:
            filepath = os.path.join(root, filename)
            excluded = False
            for pat in exclusion_patterns:
                if pat.match(filepath):
                    excluded = True
                    break
            if not excluded:
                found_files.append(filepath)
    return found_files

def read_exclusion_patterns():
    try:
        f = open(os.path.dirname(sys.argv[0]) + "/cpplint-exclusions.txt", "r")
    except IOError:
        # No such file, so assume no rules
        return []

    patterns = []
    l = f.readline().strip()
    while l != '':
        if not l.startswith('#'):
            patterns.append(re.compile(l))
        l = f.readline().strip()
    return patterns

def read_source_dirs():
    f = open(os.path.dirname(sys.argv[0]) + "/cpplint-files.txt", "r")

    lines = []
    l = f.readline().strip()
    while l != '':
        if not l.startswith("#"):
            lines.append(l)
        l = f.readline().strip()
    return lines

def main():
    exclusion_patterns = read_exclusion_patterns()
    source_dirs = read_source_dirs()

    source_files = []
    for dirname in source_dirs:
        source_files = source_files + find_files_in_directory(dirname, exclusion_patterns)

    exit_code = subprocess.call([
        './tools/cpplint.py',
        '--filter=-legal/copyright',
        '--root=controller',
        ] + source_files)
    sys.exit(exit_code)

if __name__=='__main__':
    main()
