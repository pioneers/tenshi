#!/usr/bin/env python
from __future__ import print_function

import subprocess
import sys
import os

node = subprocess.Popen(['node',
                         os.path.join(sys.argv[2],
                                      'correct_contents',
                                      'correct_contents.js'),
                         sys.argv[1]])

node.communicate()

with open('correct_contents.bin', 'rb') as f:
    buf = bytearray(f.read())
    assert len(buf) == 1024
    for index, byte in enumerate(buf):
        assert index & 0xff == byte

sys.exit(node.returncode)
