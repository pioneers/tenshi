#!/usr/bin/env python
from __future__ import print_function

import subprocess
import sys
import os

print(sys.argv)
node = subprocess.Popen(['node',
                         os.path.join(sys.argv[2],
                                      'creates_file',
                                      'creates_file.js'),
                         sys.argv[1]])

node.communicate()

with open('creates.bin'):
    pass

sys.exit(node.returncode)
