#!/usr/bin/env python

# Licensed to Pioneers in Engineering under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  Pioneers in Engineering licenses
# this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License

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
    sheet = ET.XML('''<sheet>
                        <plain> </plain>
                        <instances> </instances>
                        <busses> </busses>
                        <nets> </nets>
                      </sheet>''')
    sheets.append(sheet)
    tree.write(sys.argv[2])

if __name__ == '__main__':
    main()
