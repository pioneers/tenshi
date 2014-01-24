#!/bin/bash -xe

# CWD is still project root

mkdir -p build/boards

for board in eda/*.brd
do
	board_basename=$(basename "$board")
	outfile="build/boards/${board_basename%.*}.zip"
	./tools/run-eagle-cam-v2.py ./tools/gerber_rules.yaml $board $outfile
done
