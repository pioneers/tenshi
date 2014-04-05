# This code is used to do various things with adding version numbers to the
# final builds.

import os
import subprocess
import time


# Returns the git hash of the current head. If c_array_mode is false, returns
# a string with just the hash. If c_array_mode is true, returns a string broken
# up into bytes and with 0x prefixed to each byte.
def get_git_hash(c_array_mode):
    head_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip()
    if isinstance(head_hash, bytes):
        head_hash = head_hash.decode()
    if not c_array_mode:
        return head_hash
    rev = ""
    for i in range(0, 20):
        rev += "0x" + head_hash[i*2:i*2+2] + ", "
    return rev


# Returns True iff the working directory is considered clean (not including
# untracked files).
def get_working_dir_clean():
    git_status = subprocess.check_output(
        ['git', 'status', '--porcelain', '-uno']).strip()
    return len(git_status) == 0


# Returns True iff this is a Jenkins CI build that ends with "-committed"
def get_building_from_jenkins():
    if 'JOB_NAME' not in os.environ:
        return False
    if not os.environ['JOB_NAME'].endswith("-committed"):
        return False
    return True


# Return the current time formatted into a string
def get_build_time():
    return time.strftime('%Y%m%d%H%M%S')
