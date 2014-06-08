#!/bin/bash

gcc -Wall -m32 -fpic -shared -o fix_eagle_race_condition.so fix_eagle_race_condition.c -ldl
strip fix_eagle_race_condition.so
