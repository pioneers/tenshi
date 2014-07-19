#!/usr/bin/env python
from waf_extensions import declare_variants, run_all, sub_conf
import re
import os.path

variants = ['debug_native']
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
        target="ndnet_objects",
        includes="src"
    )
