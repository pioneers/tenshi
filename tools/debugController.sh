#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)
cd $SCRIPT_PATH/..

TENSHI_DIR=`pwd`

cd $TENSHI_DIR

export PATH="$TENSHI_DIR/../arm-toolchain/bin:${HOME}/pie/csnext/emscripten-bin:$TENSHI_DIR/../emscripten-bin/llvm/bin:$PATH"
export LLVM_DIR="$TENSHI_DIR/../emscripten-bin/llvm"
export LLVM="$LLVM_DIR/bin"



#First time only:
#sudo apt-get install libftdi1 #Install libftdi1
#cd $TENSHI_DIR/tools
#sudo cp 50-stlink.rules /etc/udev/rules.d/
#sudo restart udev


echo 'To debug code onto the Big Board:

1. Turn off the robot.
2. Find the ST-Link board in the CS Next box. (A large protoboard with a green PCB on top)
3. Connect the ST-Link board to the Big Board:
	- Tape the purple programming header with spring loaded pins onto the programming pad on the Big Board.
	- Connect the orange reset jumper to the header soldered to the reset button.
4. Connect the ST-Link board to this computer with a mini USB cable.
6. Turn on the robot.


To flash code while debugging:
1. monitor flash write_image erase ../../../../../build/artifacts/controller/deb/tenshi.bin 0x08000000'


echo
read -p "Press [Enter] to continue..."

echo
echo 'Connecting the debugger...'
echo 'Type "quit" to exit debugger.'
echo

cd $TENSHI_DIR/tools/arm-toolchain/share/openocd/scripts/
../../../bin/openocd -f ../../../../stm32f4discovery.cfg 2>/dev/null &

cd $TENSHI_DIR
./tools/arm-toolchain/bin/arm-none-eabi-gdb ./build/artifacts/controller/deb/tenshi.elf -ex 'target extended-remote localhost:3333' -ex 'monitor reset halt'


echo
echo '1. Turn robot off.
2. Unplug USB cable.
3. Disconnect the programming header and reset jumper.
4. Turn the robot back on.'

killall openocd

echo
read -p "Press [Enter] to exit."

