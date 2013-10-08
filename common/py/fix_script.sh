#!/bin/bash

echo "Shifts certain schematics in pie.lbr different amounts to fix an issue in pie.lbr caused sometime between Spring 2013 and Fall 2013."
echo "Usage: put pie.lbr in this directory with split_lbr.py, join_lbr.py, and offset_part.py"



scale=25.4

#x=$(($x*$scale))
#y=$(($y*$scale))


rm -rf fixed_split split
mkdir split

./split_lbr.py pie.lbr split

#mkdir fixed_split
cp -r split fixed_split

x=0
y=1.27 #mm = 0.05in
for n in {LED,PINHD6,PINHD8,SSOP16,XBEE-1,XT60CONNECTOR}
do
	echo "$n :"
	python offset_part.py split/$n.sym fixed_split/$n.sym 1 0 $x 0 1 $y
	echo
done

x=1.27 #mm = 0.05in
y=1.27 #mm = 0.05in
for n in PINH2X20
do
	echo "$n :"
	python offset_part.py split/$n.sym fixed_split/$n.sym 1 0 $x 0 1 $y
	echo
done

x=0.635 #mm = 0.025in
y=0
for n in {M03,M05,M07}
do
	echo "$n :"
	python offset_part.py split/$n.sym fixed_split/$n.sym 1 0 $x 0 1 $y
	echo
done

x=0.635 #mm = 0.025in
y=1.27 #mm = 0.05in
for n in {M02,M04,M06}
do
	echo "$n :"
	python offset_part.py split/$n.sym fixed_split/$n.sym 1 0 $x 0 1 $y
	echo
done


# *** M02 was fixed manually.


./join_lbr.py fixed_split pie_fixed.lbr

