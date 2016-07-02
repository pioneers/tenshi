#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)
cd $SCRIPT_PATH/..

TENSHI_DIR=`pwd`

export PATH="${HOME}/pie/csnext/arm-toolchain/bin:${HOME}/pie/csnext/emscripten-bin:${HOME}/pie/csnext/emscripten-bin/llvm/bin:$PATH"
export LLVM_DIR="${HOME}/pie/csnext/emscripten-bin/llvm"
export LLVM="$LLVM_DIR/bin"



echo 'To flash code onto the Big Board:

1. Build Tenshi
2. Turn off the robot.
3. Put a jumper on the header on the Big Board next to the programmer pad.
4. Plug a micro USB cable into the Big Board.
5. Plug the other end into this computer.
6. Turn on the robot.'

echo
#read -p "Press [Enter] to continue..."

echo
echo 'List of USB devices: (A DFU device should be here)'
lsusb | grep DFU
echo

#read -p "Press [Enter] to continue..."

echo
echo 'Flashing firmware...'
echo

cd ~/pie/csnext/tenshi/tools/arm-toolchain/share/openocd/scripts/
sudo dfu-util -d 0483:df11 -a 0 -s 0x08000000 -D ../../../../../build/artifacts/controller/deb/tenshi.bin


echo
echo '1. Turn robot off.
2. Unplug USB cable.
3. Take off the jumper.
4. Turn the robot back on.'

echo
read -p "Press [Enter] to exit."

