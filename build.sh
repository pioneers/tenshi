#!/bin/bash -xe

for dir in controller
do
	cd $dir
	./build.sh
	cd ..
done
