#!/bin/bash -xe

ANGEL_PLAYER_MAIN_DIR=$PROJECT_ROOT_DIR/angel-player

mkdir -p $PROJECT_ROOT_DIR/build/angel-player
pushd $PROJECT_ROOT_DIR/build/angel-player

# Download XULRunner if it isn't already
if [ ! -e xulrunner-27.0.1.en-US.linux-x86_64.tar.bz2 ]
then
    if [ -e ~/tenshi-cache/xulrunner-27.0.1.en-US.linux-x86_64.tar.bz2 ]
    then
        cp ~/tenshi-cache/xulrunner-27.0.1.en-US.linux-x86_64.tar.bz2 .
    else
        wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/27.0.1/runtimes/xulrunner-27.0.1.en-US.linux-x86_64.tar.bz2
    fi
fi
if [ ! -e xulrunner-27.0.1.en-US.win32.zip ]
then
    if [ -e ~/tenshi-cache/xulrunner-27.0.1.en-US.win32.zip ]
    then
        cp ~/tenshi-cache/xulrunner-27.0.1.en-US.win32.zip .
    else
        wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/27.0.1/runtimes/xulrunner-27.0.1.en-US.win32.zip
    fi
fi
if [ ! -e xulrunner-27.0.1.en-US.mac.tar.bz2 ]
then
    if [ -e ~/tenshi-cache/xulrunner-27.0.1.en-US.mac.tar.bz2 ]
    then
        cp ~/tenshi-cache/xulrunner-27.0.1.en-US.mac.tar.bz2 .
    else
        wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/27.0.1/runtimes/xulrunner-27.0.1.en-US.mac.tar.bz2
    fi
fi

# Prepare linux version and windows version. Use an awful hack to combine them.
rm -rf angel-player-win32-lin64
mkdir angel-player-win32-lin64
pushd angel-player-win32-lin64
unzip ../xulrunner-27.0.1.en-US.win32.zip
mv xulrunner xul-win32
tar xjf ../xulrunner-27.0.1.en-US.linux-x86_64.tar.bz2
mv xulrunner xul-lin64
cp xul-lin64/xulrunner-stub angel-player
cp xul-win32/xulrunner-stub.exe angel-player.exe
# This is a god-awful hack to change the XULrunner directory. This essentially
# patches line 261 of nsXULStub.cpp
sed -i 's/%sxulrunner/%sxul-lin64/g' angel-player
sed -i 's/%sxulrunner/%sxul-win32/g' angel-player.exe
# Copy in angel-player code
cp -r --dereference $ANGEL_PLAYER_MAIN_DIR/src/* .
# Remove debug file
rm defaults/preferences/debug.js
popd

# Prepare mac version
rm -rf angel-player-mac.app
mkdir angel-player-mac.app
pushd angel-player-mac.app
# Mac OS requires a bunch of random futzing with stuff
mkdir -p Contents/Frameworks
mkdir -p Contents/Resources
mkdir -p Contents/MacOS
pushd Contents/Frameworks
tar xjf ../../../xulrunner-27.0.1.en-US.mac.tar.bz2
popd
cp -r --dereference $ANGEL_PLAYER_MAIN_DIR/src/* Contents/Resources
# Remove debug file
rm Contents/Resources/defaults/preferences/debug.js
cp -r $ANGEL_PLAYER_MAIN_DIR/meta-mac/* Contents
popd

$PROJECT_ROOT_DIR/tools/inject-version-angel-player.py $ANGEL_PLAYER_MAIN_DIR/src/application.ini angel-player-win32-lin64/application.ini angel-player-mac.app/Contents/Resources/application.ini

# Archive the outputs
tar cjf angel-player-win32-lin64.tar.bz2 angel-player-win32-lin64 &
tar cjf angel-player-mac.tar.bz2 angel-player-mac.app &

# Make sure archiving finishes
wait

popd
