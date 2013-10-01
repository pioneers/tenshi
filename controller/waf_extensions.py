from waflib.Build import BuildContext, CleanContext, \
    InstallContext, UninstallContext


def declare_variants(*args):
    for x in args:
        for y in (BuildContext, CleanContext, InstallContext,
                  UninstallContext):
            name = y.__name__.replace('Context', '').lower()
            # Examples of name: build, clean, install...

            class tmp(y):
                cmd = name + '_' + x
                variant = x
