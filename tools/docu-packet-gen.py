#!/usr/bin/env python

from __future__ import print_function
import os
import os.path
import subprocess
import sys
import shutil
from generate_bom import start_xvfb, kill_xvfb
from generate_bom import get_eagle_path, setup_tmp_dir, remove_tmp_dir


def run_script(file_name, script_name):
    ret = subprocess.call([
        get_eagle_path(),
        file_name,
        '-S' + script_name,
        ]
        )

    if ret != 0:
        print("Eagle returned error!")
        sys.exit(ret)


def copy_and_replace(src, dst, pattern, replacement):
    with open(src) as src_file:
        with open(dst, "w") as dst_file:
            dst_file.write(src_file.read().replace(pattern, replacement))


def compile_pdf(sch_img, top_img, bottom_img, pdf):
    ret = subprocess.call([
        "pdftk",
        sch_img,
        top_img,
        bottom_img,
        "cat",
        "output",
        pdf
        ]
        )

    if ret != 0:
        print("Eagle returned error!")
        sys.exit(ret)


def main():
    if len(sys.argv) < 3:
        print("Usage: %s in.sch|in.brd out.pdf" % (sys.argv[0]))
        sys.exit(1)

    scr_dir = os.path.abspath(os.path.dirname(sys.argv[0]))

    base_name = os.path.splitext(os.path.abspath(sys.argv[1]))[0]
    out_name = os.path.join(os.getcwd(), os.path.abspath(sys.argv[2]))

    sch_name = os.path.join(os.getcwd(), base_name + ".sch")
    brd_name = os.path.join(os.getcwd(), base_name + ".brd")

    # Start xvfb
    xvfb, display_num = start_xvfb()

    # Create temporary directory
    tmp_dir = setup_tmp_dir()

    # Copy scripts to the temporary directory
    # Eagle's default location for saving exported images is unrelated to the
    # current working directory, so the scripts must be modified to hardcode
    # the output file paths
    copy_and_replace(os.path.join(scr_dir, "docu-packet-schematic.scr"),
                     os.path.join(tmp_dir, "schematic.scr"),
                     "%PATH%",
                     tmp_dir)
    copy_and_replace(os.path.join(scr_dir, "docu-packet-board.scr"),
                     os.path.join(tmp_dir, "board.scr"),
                     "%PATH%",
                     tmp_dir)

    # Generate schematic image
    dst_sch_name = os.path.join(tmp_dir, "file.sch")
    shutil.copy(sch_name, dst_sch_name)
    run_script(dst_sch_name, "schematic.scr")
    os.remove(dst_sch_name)

    # Generate board images
    dst_brd_name = os.path.join(tmp_dir, "file.brd")
    shutil.copy(brd_name, dst_brd_name)
    run_script(dst_brd_name, "board.scr")
    os.remove(dst_brd_name)

    # Compile final pdf
    compile_pdf(os.path.join(tmp_dir, "schematic.pdf"),
                os.path.join(tmp_dir, "top.pdf"),
                os.path.join(tmp_dir, "bottom.pdf"),
                out_name)

    # Clean up
    remove_tmp_dir(tmp_dir)
    kill_xvfb(xvfb)

if __name__ == '__main__':
    main()
