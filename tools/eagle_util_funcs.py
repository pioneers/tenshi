from __future__ import print_function

import os
import shutil
import signal
import subprocess
import sys
import tempfile
import time


XVFB_START_TIMEOUT = 2  # Seconds


def start_xvfb():
    display_num = os.getpid()
    xvfb_process = subprocess.Popen([
        'Xvfb',
        ':%d' % display_num,
        '-screen', '0', '1024x768x24',
        ])
    start_time = time.time()
    found_x_socket = False
    while time.time() - start_time < XVFB_START_TIMEOUT:
        if os.path.exists("/tmp/.X11-unix/X%d" % display_num):
            found_x_socket = True
            break
    if not found_x_socket:
        print("Timeout waiting for Xvfb!")
        sys.exit(1)
    print("Spawned Xvfb, display %d, pid %d" %
          (display_num, xvfb_process.pid))
    os.environ["DISPLAY"] = ":%d" % display_num
    return (xvfb_process, display_num)


def kill_xvfb(xvfb_process):
    xvfb_process.send_signal(signal.SIGINT)
    xvfb_process.wait()
    print("Killed Xvfb")


_eagle_binary_path = '/opt/eagle-6.5.0/bin/eagle'


def get_eagle_path():
    global _eagle_binary_path
    if not os.path.exists(_eagle_binary_path):
        which_eagle = subprocess.Popen(['which', 'eagle'],
                                       stdout=subprocess.PIPE)
        which_eagle.wait()
        status = which_eagle.returncode
        if status != 0:
            print("Could not find eagle!")
            sys.exit(1)
        else:
            # Remove newline from which ouput.
            _eagle_binary_path = which_eagle.stdout.read().strip()
    return _eagle_binary_path


def run_eagle(args):
    # Hack to prevent .eaglerc race condition
    fix_eagle_so = os.path.abspath(
        os.path.dirname(__file__) + "/fix_eagle_race_condition.so")
    os.environ["LD_PRELOAD"] = fix_eagle_so

    ret = subprocess.call([get_eagle_path()] + args)

    del os.environ["LD_PRELOAD"]

    return ret


def setup_tmp_dir():
    tmpdir = tempfile.mkdtemp()
    print("Files in %s" % tmpdir)
    os.chdir(tmpdir)
    return tmpdir


def remove_tmp_dir(tmpdir):
    shutil.rmtree(tmpdir)
