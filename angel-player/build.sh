#!/bin/bash -xe

mkdir -p build
cd build

# Download XULRunner if it isn't already
if [ ! -e xulrunner-22.0.en-US.linux-x86_64.tar.bz2 ]
then
    wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/22.0/runtimes/xulrunner-22.0.en-US.linux-x86_64.tar.bz2
fi
if [ ! -e xulrunner-22.0.en-US.win32.zip ]
then
    wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/22.0/runtimes/xulrunner-22.0.en-US.win32.zip
fi
# Mac OS X version is disabled right now because XULRunner seems to be broken
# https://bugzilla.mozilla.org/show_bug.cgi?id=738790
# if [ ! -e xulrunner-22.0.en-US.mac.tar.bz2 ]
# then
#     wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/22.0/runtimes/xulrunner-22.0.en-US.mac.tar.bz2
# fi

# Prepare linux version
rm -rf angel-player-linux-x86_64
mkdir angel-player-linux-x86_64
cd angel-player-linux-x86_64
tar xjf ../xulrunner-22.0.en-US.linux-x86_64.tar.bz2
cp xulrunner/xulrunner-stub angel-player
cp -r ../../src/* .
cd ..

# Prepare windows version
rm -rf angel-player-win32
mkdir angel-player-win32
cd angel-player-win32
unzip ../xulrunner-22.0.en-US.win32.zip
cp xulrunner/xulrunner-stub.exe angel-player.exe
cp -r ../../src/* .
cd ..

# Prepare mac version
# rm -rf angel-player-mac.app
# mkdir angel-player-mac.app
# cd angel-player-mac.app
# # Mac OS requires a bunch of random futzing with stuff
# mkdir -p Contents/Frameworks
# mkdir -p Contents/Resources
# mkdir -p Contents/MacOS
# pushd Contents/Frameworks
# tar xjf ../../../xulrunner-22.0.en-US.mac.tar.bz2
# popd
# cp Contents/Frameworks/XUL.framework/Versions/Current/xulrunner Contents/MacOS/xulrunner
# cp -r ../../src/* Contents/Resources
# cp -r ../../meta-mac/* Contents
# cd ..

../inject-version.py ../src/application.ini angel-player-linux-x86_64/application.ini angel-player-win32/application.ini

# Archive the outputs
tar cjf angel-player-linux-x86_64.tar.bz2 angel-player-linux-x86_64
tar cjf angel-player-win32.tar.bz2 angel-player-win32
# tar cjf angel-player-mac.tar.bz2 angel-player-mac.app

cd ..
