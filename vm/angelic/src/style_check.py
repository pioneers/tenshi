#!/usr/bin/env python2

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

from __future__  import print_function

def check(file_stream):
    lines = file_stream.readlines()
    current_defs = set()
    for line in lines:
        # print(current_defs)
        s_line = line.strip()
        if s_line.startswith('#define'):
            name = s_line.split(' ')[1].partition('(')[0]
            #print('Define: {0}'.format(name))
            current_defs.add(name)
        elif s_line.startswith('#ifdef'):
            name = s_line.split(' ')[1].partition('(')[0]
            # print('Ifdef: {0}'.format(name))
            current_defs.add(name)
        elif s_line.startswith('#ifndef'):
            name = s_line.split(' ')[1].partition('(')[0]
            # print('Ifndef: {0}'.format(name))
            current_defs.add(name)
        elif s_line.startswith('#undef'):
            try:
                name = s_line.split(' ')[1]
                #print('Undef: {0}'.format(name))
                current_defs.remove(name)
            except KeyError:
                print ('#undefed unknown macro {}'
                        .format(s_line.split(' ')[1]))
                pass
    if current_defs:
        print('Style error, {0} still #defined.'.format(', '.join(current_defs)))

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Check a C file for specific errors')
    parser.add_argument('file')
    args = parser.parse_args()
    with open(args.file) as the_file:
        check(the_file)
