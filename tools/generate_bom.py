#!/usr/bin/env python
from __future__ import print_function

import os
import os.path
import shutil
import subprocess
import sys
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


def kill_xvfb(xvfb_process):
    xvfb_process.kill()
    print("Killed Xvfb")


def setup_tmp_dir():
    tmpdir = tempfile.mkdtemp()
    print("Files in %s" % tmpdir)
    os.chdir(tmpdir)
    return tmpdir


def remove_tmp_dir(tmpdir):
    shutil.rmtree(tmpdir)


def run_eagle_bom_ulp(infile, outfile, ulpfile):
    subprocess.call([
        '/opt/eagle-6.5.0/bin/eagle',
        '-C', 'run %s %s; quit' % (ulpfile, outfile),
        infile,
        ])


def main():
    if len(sys.argv) < 3:
        print("Usage: %s schematic.sch bom.csv" % sys.argv[0])
        sys.exit(1)

    ulpFile = os.path.abspath(os.path.dirname(sys.argv[0]) + "/tenshi-bom.ulp")
    schematicPath = os.path.abspath(sys.argv[1])
    outfilePath = os.path.abspath(sys.argv[2])

    # Start up Xvfb
    xvfb, display_num = start_xvfb()

    # Create temp directory (work around EAGLE running the ULP in BRD instead
    # of in the SCH). Also will chdir into it.
    tmpdir = setup_tmp_dir()
    shutil.copyfile(schematicPath, "input.sch")

    # Run the BOM-generating ULP
    run_eagle_bom_ulp("input.sch", "bom-temp.csv", ulpFile)

    # TODO(rqou): There should probably be some error checking somewhere here

    # TODO(rqou): Actual postprocessing
    # Copy the output file
    shutil.copyfile('bom-temp.csv', outfilePath)

    # Cleanup
    remove_tmp_dir(tmpdir)
    kill_xvfb(xvfb)

if __name__ == '__main__':
    main()
