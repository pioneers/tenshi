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

which emcc >/dev/null || {
	# Extract emscripten
	if [ ! -e emscripten-bin ]
	then
		if [ ! -e emscripten-bin-18apr2014.tar.bz2 ]
		then
			wget http://rqou.com/emscripten-bin-18apr2014.tar.bz2
		fi
		tar xjf emscripten-bin-18apr2014.tar.bz2
	fi
}

cd ..
