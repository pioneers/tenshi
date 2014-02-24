#!/usr/bin/env python
from __future__ import print_function

import os
import subprocess as subp
import shutil


def is_subpath_of_set(path, pathset):
    part_path = ''
    for part in path.split(os.path.sep):
        part_path = os.path.join(part_path, part)
        if part_path in pathset:
            return True
    return False


def setup():
    os.chdir('tests')
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


EXT_TO_CMD = {'py': 'python', 'js': 'node', 'sh': 'bash'}


def get_tests():
    '''
    Get all files under the tests directory, skipping directories that have
    tests files with the same name.
    '''
    tests = set()
    dirs_to_skip = set()
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
    p = subp.Popen([cmd,
                    get_test(name),
                    root,
                    get_test_base(root, name)
                    ],
                   stdout=subp.PIPE,
                   stderr=subp.STDOUT)
    output = p.communicate()
    ret = p.returncode
    if ret == 0:
        print('.', end='')
    else:
        failed_tests.append(name)
        stdout_logs[name] = output
        print('x', end='')


def run_all():
    tests = get_tests()
    failed_tests = []
    stdout_logs = dict()
    root = os.getcwd()

    setup()

    try:
        for test in sorted(tests):
            run_test(test, root, failed_tests, stdout_logs)
    except Exception as e:
        print('Encountered exception while running tests', e)
    finally:
        cleanup()

    if not failed_tests:
        print()
        print('OK (Ran {0} test)'.format(len(tests)))
    else:
        print()
        for failure in failed_tests:
            print('FAILED:', failure)
            print(' BEGIN TEST OUTPUT '.center(80, '*'))
            print(stdout_logs[failure][0].decode(), end='')
            print(' END TEST OUTPUT '.center(80, '*'))
            print()
        print('TEST FAILED ({0}/{1} tests failed)'
              .format(len(failed_tests), len(tests)))


if __name__ == '__main__':
    run_all()
