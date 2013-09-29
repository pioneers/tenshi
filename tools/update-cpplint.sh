#!/bin/bash -xe

# Run this from the 'tools' directory

wget http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
chmod +x cpplint.py
patch < cpplint.patch
