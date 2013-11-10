#!/usr/bin/env python

from __future__ import print_function
import os.path
import shutil
import sys
import subprocess
import tempfile
import time


def start_xvfb():
    display_num = os.getpid()
    xvfb_process = subprocess.Popen([
        'Xvfb',
        ':%d' % display_num,
        '-screen', '0', '1024x768x24',
        ])
    time.sleep(2)
    print("Spawned Xvfb, display %d, pid %d" %
         (display_num, xvfb_process.pid))
    os.environ["DISPLAY"] = ":%d" % display_num
    return (xvfb_process, display_num)


def start_awesome():
    awesome_rc_file = os.path.dirname(sys.argv[0]) + "/minimal-rc.lua"
    awesome_process = subprocess.Popen([
        'awesome',
        '-c', awesome_rc_file,
        ])
    time.sleep(0.5)
    print("Spawned awesome, pid %d" % awesome_process.pid)
    return awesome_process


def kill_awesome(awesome_process):
    awesome_process.kill()
    print("Killed awesome")


def kill_xvfb(xvfb_process):
    xvfb_process.kill()
    print("Killed Xvfb")


def setup_tmp_dir():
    tmpdir = tempfile.mkdtemp()
    print("Files in %s" % tmpdir)
    os.mkdir(tmpdir + "/gerb")
    os.chdir(tmpdir)
    return tmpdir


def remove_tmp_dir(tmpdir):
    shutil.rmtree(tmpdir)


def spawn_eagle(camfile):
    eagle_process = subprocess.Popen([
        '/opt/eagle-6.5.0/bin/eagle',
        camfile,
        ])
    time.sleep(0.5)
    print("Spawned eagle, pid %d" % eagle_process.pid)
    return eagle_process


def open_board(brdfile):
    print("Starting to type!")
    # File->Open->Board
    subprocess.call([
        'xdotool',
        'search', 'CAM Processor',
        'windowactivate', '--sync',
        # TODO(rqou): Why does it not work without this delay?
        'key', '--clearmodifiers', '--delay', '500', 'alt+f', 'o', 'b',
        ])
    time.sleep(1)
    # Filename
    subprocess.call([
        'xdotool',
        'type', '--clearmodifiers', brdfile,
        ])
    time.sleep(1)
    # Acknowlege the file open dialog
    subprocess.call([
        'xdotool',
        'key', '--clearmodifiers', 'KP_Enter',
        ])
    time.sleep(1)
    # Start
    subprocess.call([
        'xdotool',
        'key', '--clearmodifiers', 'alt+j',
        ])
    time.sleep(1)
    print("Input sent!")


def supervise_cam():
    while True:
        # Look for the progress window
        progress_open = subprocess.call([
            'xdotool',
            'search', '--all', '--name', '--class', '^eagle$',
            ])
        if progress_open != 0:
            print("Progress bar gone!")
            # Exit
            subprocess.call([
                'xdotool',
                'key', '--clearmodifiers', 'alt+x',
                ])
            time.sleep(1)
            return True

        # Look for the warning window
        warning_open = subprocess.call([
            'xdotool',
            'search', '--name', 'Warning',
            ])
        if warning_open == 0:
            print("Eagle issued a CAM warning")
            # yes to all
            subprocess.call([
                'xdotool',
                'key', '--clearmodifiers', 'a',
                ])
            time.sleep(1)

        # Look for the error window
        error_open = subprocess.call([
            'xdotool',
            'search', '--name', 'Error',
            ])
        if error_open == 0:
            print("An error occurred!")
            return False

        print("Waiting...")
        time.sleep(1)


def zip_up_results(outfile):
    subprocess.call("cd gerb && zip %s *" % outfile, shell=True)
    print("Made zip!")


def main():
    if len(sys.argv) < 3:
        print("Usage: %s job.cam board.brd out.zip" % sys.argv[0])
        sys.exit(1)

    camfile = os.path.abspath(sys.argv[1])
    brdfile = os.path.abspath(sys.argv[2])
    outfile = os.path.abspath(sys.argv[3])

    print(camfile, brdfile, outfile)

    # Start up X11/WM stuff
    xvfb, display_num = start_xvfb()
    awesome = start_awesome()

    # Set up output directory
    tmpdir = setup_tmp_dir()

    # Run CAM
    spawn_eagle(camfile)
    open_board(brdfile)
    # TODO(rqou): Hack
    time.sleep(5)
    worked = supervise_cam()

    if worked:
        # Zip the results
        zip_up_results(outfile)

    # Cleanup now that it's done
    remove_tmp_dir(tmpdir)
    kill_awesome(awesome)
    kill_xvfb(xvfb)

    if not worked:
        sys.exit(1)

if __name__ == '__main__':
    main()
