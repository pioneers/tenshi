#!/usr/bin/env python

from __future__ import print_function
import sys
import xml.etree.ElementTree as ET


def main():
    if len(sys.argv) != 3:
        print("Usage: {} <infile> <outfile>".format(sys.argv[0]))
        sys.exit(1)

    tree = ET.parse(sys.argv[1])
    root = tree.getroot()
    sheets = root.find('drawing/schematic/sheets')
    sheet = ET.XML('''<sheet> <plain> </plain> <instances> </instances> <busses> </busses> <nets> </nets> </sheet>''')
    sheets.append(sheet)
    tree.write(sys.argv[2])

if __name__ == '__main__':
    main()
