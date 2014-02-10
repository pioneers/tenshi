from collections import defaultdict
from waflib.Build import BuildContext, CleanContext, \
    InstallContext, UninstallContext

variants = defaultdict(set)
cmds = defaultdict(set)

def declare_variants(args=None, subdir=''):
    if not args:
        args = ['']
    for x in args:
        var = x
        if subdir != '':
            var = subdir + '/' + x
        for y in (BuildContext, CleanContext, InstallContext, UninstallContext):
            name = y.__name__.replace('Context','').lower()
            # Examples of name: build, clean, install...
            c = name + '_' + var
            cmds[name].add(c)
            class tmp(y):
                cmd = c
                variant = var
            variants[x].add(tmp)

def run_all(kind):
    from waflib.Scripting import run_command
    for cmd in cmds[kind]:
        run_command(cmd)

def clone_env(conf, old_name, new_name):
    conf.setenv(old_name)
    new_env = conf.env.derive()
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
