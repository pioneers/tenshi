#!/bin/bash -xe

for dir in controller angel-player
do
	cd $dir
	./build.sh
	cd ..
done
