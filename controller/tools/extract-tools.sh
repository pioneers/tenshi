#!/bin/bash -xe

cd tools

# Extract the cross-gcc
if [ ! -e arm-toolchain ]
then
	tar xjf project-tenshi-toolchain.tar.bz2
	# OpenOCD is not strictly necessary for building
	if [ -e project-tenshi-openocd.tar.bz2 ]
	then
		tar xjf project-tenshi-openocd.tar.bz2
	fi
fi

cd ..
