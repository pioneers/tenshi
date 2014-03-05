#!/usr/bin/env python2
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
