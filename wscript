from __future__ import print_function

def recurse(ctx):
    ctx.recurse('eda')
    ctx.recurse('controller')

def options(ctx):
    recurse(ctx)

def configure(ctx):
    recurse(ctx)

def build(ctx):
    recurse(ctx)
