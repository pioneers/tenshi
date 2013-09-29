#!/bin/bash -xe

cd tools

# Extract the cross-gcc
if [ ! -e arm-toolchain ]
then
	if [ ! -e project-tenshi-toolchain.tar.bz2 ]
	then
		wget https://rqou.com/project-tenshi-tools/project-tenshi-toolchain.tar.bz2
	fi
	tar xjf project-tenshi-toolchain.tar.bz2

	if [ ! -e project-tenshi-openocd.tar.bz2 ]
	then
		wget https://rqou.com/project-tenshi-tools/project-tenshi-openocd.tar.bz2
	fi
	tar xjf project-tenshi-openocd.tar.bz2
fi

cd ..
