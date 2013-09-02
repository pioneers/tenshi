#!/bin/bash -xe

mkdir -p build
cd build

# Download XULRunner if it isn't already
if [ ! -e xulrunner-22.0.en-US.linux-x86_64.tar.bz2 ]
then
	wget ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/22.0/runtimes/xulrunner-22.0.en-US.linux-x86_64.tar.bz2
fi

rm -rf angel-player-linux-x86_64
mkdir angel-player-linux-x86_64
cd angel-player-linux-x86_64
tar xjf ../xulrunner-22.0.en-US.linux-x86_64.tar.bz2
cp xulrunner/xulrunner-stub angel-player
cp -r ../../src/* .
cd ..
tar cjf angel-player-linux-x86_64.tar.bz2 angel-player-linux-x86_64

cd ..
