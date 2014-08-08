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

from collections import defaultdict
import waflib.Build

from waflib import Utils
import os.path
import re

variants = defaultdict(set)
cmds = defaultdict(set)


def declare_variants(args=None, subdir=''):
    if not args:
        args = ['']
    for a in args:
        var = a
        if subdir != '':
            var = subdir + '/' + a
        ctxts = []
        ctxt_re = re.compile('(\w+)Context')
        for key, ctxt in waflib.Build.__dict__.items():
            m = ctxt_re.match(key)
            if m is not None:
                t = m.group(1).lower()
                cmd_name = '{0}_{1}'.format(t, var)
                cmds[t].add(cmd_name)

                class new_variant(ctxt):
                    cmd = cmd_name
                    variant = var

                variants[a].add(new_variant)


def run_all(kind):
    from waflib.Scripting import run_command
    for cmd in cmds[kind]:
        run_command(cmd)


def clone_env(conf, old_name, new_name):
    conf.setenv(old_name)
    new_env = conf.env.copy()
    new_env.detach()
    conf.setenv(new_name, new_env)


class sub_conf(object):
    def __init__(self, conf, name):
        if conf.variant != '':
            name = conf.variant + '/' + name
        self.conf = conf
        self.name = name
        self.old_env = conf.variant

    def __enter__(self):
        new_env = self.conf.env.derive()
        new_env.detach()
        self.conf.setenv(self.name, new_env)

    def __exit__(self, *args):
        self.conf.setenv(self.old_env)


def add_dependency(bld, tgt, src):
    # Expand configuration variables (e.g.
    # "${root}/tools" -> "/home/.../tenshi/tools")
    tgt = Utils.subst_vars(tgt, bld.env)
    src = Utils.subst_vars(src, bld.env)
    # Handle paths relative the current wscript
    src_node = bld.path.find_resource(src)
    if src_node is None:
        # Handle paths relative to the root and absolute paths.
        src_node = bld.root.find_resource(src)
    if src_node is None:
        bld.fatal("Could not find manual dependency '{}'".format(src))
    bld.add_manual_dependency(tgt, src_node)


def configure_emscripten(conf):
    conf.env['CC'] = 'emcc'
    conf.env['CC_NAME'] = 'emcc'
    conf.env['COMPILER_CC'] = 'emcc'
    conf.env['LINK_CC'] = 'emcc'
    conf.env.append_value('CFLAGS', '-Wno-warn-absolute-paths')
    conf.load('compiler_c')
    conf.env['cprogram_PATTERN'] = '%s.js'
    conf.env['AR'] = 'llvm-ar'
    conf.load('ar')


def export_str_from_filenames(filenames):
    func_decl = re.compile(r'''
    # Begin at the start of the line
    ^
    # Do not capture the next group
    (?:
    # Require some words with possible * and ().
    [*()]*\w+[*()]*\s+
    # There must be at least one such word
    )+
    # Then allow * and (
    [*(]*
    # There may be some whitespace
    \s*
    # Capture the function name.
    (\w+)
    # There may be some whitespace
    \s*
    # Allow a )
    \)?
    # There may be some whitespace
    \s*
    # And then an argument list
    \(''', flags=re.MULTILINE | re.VERBOSE)

    api_fns = []

    for filename in filenames:
        with open(filename) as f:
            contents = f.read()
            api_fns.extend(func_decl.findall(contents))

    export_str = ','.join(
        "'_{}'".format(fn) for fn in api_fns)
    return export_str


def build_emcc_lib(bld, export_str, target, **kwargs):

    fake_node_js = os.path.join(bld.env['root'],
                                'vm/angelic/src/fake_node.js')

    # Using emscripten, build a .so, and from that build a .js.
    bld(
        features=['c', 'cshlib'],
        target=target,
        **kwargs
    )
    bld(
        name='emcc library',
        rule=' '.join([
            'emcc ${SRC} -o ${TGT} ${CFLAGS}',
            '-s EXPORTED_FUNCTIONS="[' + export_str + ']"',
            '--llvm-lto 3',
            '-s RESERVED_FUNCTION_POINTERS=1024',
            '-s TOTAL_STACK=1600000',
            '--pre-js ' + fake_node_js,
            ]),
        source='lib{0}.so'.format(target),
        target='{0}.js'.format(target),
    )
    bld.add_manual_dependency('{0}.js'.format(target),
                              fake_node_js)
    bld.add_manual_dependency('{0}.js'.format(target),
                              target)
