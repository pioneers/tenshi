#!/bin/bash -xe

cd tools

# Extract the cross-gcc
if [ ! -e arm-toolchain ]
then
	tar xjf project-tenshi-toolchain.tar.bz2
fi

cd ..
