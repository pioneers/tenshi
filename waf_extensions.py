from collections import defaultdict
import waflib.Build

from waflib import Utils
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
