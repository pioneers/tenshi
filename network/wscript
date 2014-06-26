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

from waf_extensions import declare_variants, run_all, sub_conf, \
    build_emcc_lib, export_str_from_filenames, \
    configure_emscripten
import re
import os.path

variants = ['release_emscripten', 'debug_native']
declare_variants(variants,
                 subdir='network')


def options(opt):
    opt.load('compiler_c')


def configure_debug_native(conf):
    conf.load('compiler_c')
    conf.load('ar')
    conf.env.append_value('CFLAGS', '-std=gnu99')
    conf.env.append_value('CFLAGS', '-g')


def configure(conf):
    if not conf.env['root']:
        conf.env['root'] = os.path.join(conf.path.abspath(), os.pardir,
                                        os.pardir)
    with sub_conf(conf, 'network/release_emscripten'):
        configure_emscripten(conf)
    with sub_conf(conf, 'network/debug_native'):
        configure_debug_native(conf)


def build(bld):
    if not bld.variant:
        print('Building all variants.')
        run_all('build')
        return
    # Build for the controller and lua itself.
    # Currently, lua is only built on its own using emscripten.
    if 'controller' not in bld.variant and 'network' not in bld.variant:
        return

    # We want to glob all files in the src directory which don't contain main
    # functions.
    main_re = re.compile(r'^ *int +main *\(', re.MULTILINE)
    paths = bld.path.ant_glob("src/**/*.c")
    files = []

    # Filter out files which contain a main function
    for path in paths:
        with open(path.abspath()) as f:
            contents = f.read()
            if not main_re.search(contents):
                files.append(path)

    # Always build the objects.
    bld.objects(
        source=files,
        target="ndl3_objects",
        includes="src"
    )

    if 'native' in bld.variant or 'emscripten' in bld.variant:
        bld.program(
            source='src/test.c',
            target='ndl3_test',
            use='ndl3_objects',
            includes='src',
        )

    if 'emscripten' in bld.variant:
        header = os.path.join(bld.env['root'], 'network', 'src', 'ndl3.h')

        export_str = export_str_from_filenames([header])

        build_emcc_lib(bld, export_str, target='ndl3', use='ndl3_objects')
