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

# Check if any changes were made to the code
find -L $ANGEL_PLAYER_MAIN_DIR/src -type f -exec md5sum {} \; > new-src-hash
find -L $ANGEL_PLAYER_MAIN_DIR/meta-mac -type f -exec md5sum {} \; >> new-src-hash

if [ ! -e src-hash ]
then
    should_rearchive=1
else
    # We need to use diff rather than md5sum -c or similar in order to find
    # new/deleted files
    set +e
    diff -a new-src-hash src-hash >/dev/null
    if [ $? -eq 0 ]
    then
        # No difference
        should_rearchive=0
    elif [ $? -eq 1 ]
    then
        # Difference
        should_rearchive=1
    else
        echo "Diff failed!"
        exit $?
    fi
    set -e
fi

if [ $should_rearchive -eq 0 ]
then
    echo "Skipping build due to no files changed..."
    exit 0
fi

cp new-src-hash src-hash

# Prepare Mac/Linux/Windows version. Use an awful hack to combine them.
rm -rf angel-player.app
mkdir angel-player.app
pushd angel-player.app
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
# Do Mac stuff
# Mac OS requires a bunch of random futzing with stuff, but it's all in its
# own directory.
mkdir -p Contents/Frameworks
pushd Contents/Frameworks
tar xjf ../../../xulrunner-27.0.1.en-US.mac.tar.bz2
popd
# Symlink the source code. Breaks on Windows. Whatever.
ln -s .. Contents/Resources
# Random plists and stub and things.
cp -r $ANGEL_PLAYER_MAIN_DIR/meta-mac/* Contents
popd

$PROJECT_ROOT_DIR/tools/inject-version-angel-player.py $ANGEL_PLAYER_MAIN_DIR/src/application.ini angel-player.app/application.ini

# Archive the output
tar cjf ../artifacts/angel-player.tar.bz2 angel-player.app

popd
