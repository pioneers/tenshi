#!/usr/bin/env python
from __future__ import print_function

import os
import subprocess as subp
import shutil
import argparse
import re
import traceback
import sys


def is_subpath_of_set(path, pathset):
    part_path = ''
    for part in path.split(os.path.sep):
        part_path = os.path.join(part_path, part)
        if part_path in pathset:
            return True
    return False


def setup():
    os.chdir('tests')
    try:
        os.mkdir('tmp')
    except IOError as e:
        # One reason we would get here is that the tmp directory was not
        # cleaned up. Delete the directory and try again.
        shutil.rmtree('tmp')
        os.mkdir('tmp')
    os.chdir('tmp')


def cleanup():
    os.chdir(os.pardir)
    shutil.rmtree('tmp')
    os.chdir(os.pardir)


def get_test(path):
    return os.path.join(os.pardir, os.pardir, path)


def get_test_base(root, path):
    return os.path.join(root, os.path.dirname(path))


EXT_TO_CMD = {'py': ['python'], 'js': ['node', '--harmony'], 'sh': ['bash']}


def get_tests():
    '''
    Get all files under the tests directory, skipping directories that have
    tests files with the same name.
    '''
    tests = set()
    dirs_to_skip = set(['tests/tmp'])
    for dirpath, dirnames, filenames in os.walk('tests', topdown=True):
        if not is_subpath_of_set(dirpath, dirs_to_skip):
            for filename in filenames:
                if filename[0] == '.':
                    continue
                fullname = os.path.join(dirpath, filename)
                tests.add(fullname)
                base, ext = os.path.splitext(fullname)
                dirs_to_skip.add(base)
    return tests


def run_test(name, root, failed_tests, stdout_logs):
    _, ext = os.path.splitext(name)
    cmd = EXT_TO_CMD[ext[1:]]
    args = cmd + [get_test(name),
                  root,
                  get_test_base(root, name)]
    p = subp.Popen(args,
                   stdout=subp.PIPE,
                   stderr=subp.STDOUT)
    stdout_logs[name] = p.communicate()
    ret = p.returncode
    if ret == 0:
        print('.', end='')
    else:
        failed_tests.append(name)
        print('x', end='')
    sys.stdout.flush()


def main():
    parser = argparse.ArgumentParser(description='Run angelic tests.')
    parser.add_argument('--no-cleanup', action='store_true')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--matching', action='store', default='')
    args = parser.parse_args()
    pattern = None
    if args.matching:
        pattern = re.compile(args.matching)

    tests = get_tests()
    failed_tests = []
    stdout_logs = dict()
    root = os.getcwd()

    tests_run = 0

    setup()

    try:
        for test in sorted(tests):
            if not pattern or pattern.search(test):
                tests_run += 1
                try:
                    run_test(test, root, failed_tests, stdout_logs)
                except KeyboardInterrupt as e:
                    print('Encountered exception while running test{}:'
                          .format(test))
                    traceback.print_exc()
                if args.verbose:
                    print(stdout_logs[test][0].decode())
    except Exception as e:
        print('Encountered exception while running tests:')
        traceback.print_exc()
    finally:
        if not args.no_cleanup:
            cleanup()

    if not failed_tests:
        print()
        print('OK (Ran {0} test)'.format(tests_run))
    else:
        print()
        for failure in failed_tests:
            print('FAILED:', failure)
            print(' BEGIN TEST OUTPUT '.center(80, '*'))
            print(stdout_logs[failure][0].decode(), end='')
            print(' END TEST OUTPUT '.center(80, '*'))
            print()
        print('TEST FAILED ({0}/{1} tests failed)'
              .format(len(failed_tests), tests_run))


if __name__ == '__main__':
    main()
