ARM_CROSS_COMPILER = 'arm-none-eabi-gcc'

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.env['CC'] = ARM_CROSS_COMPILER
    conf.env['CC_NAME'] = ARM_CROSS_COMPILER
    conf.env['COMPILER_CC'] = ARM_CROSS_COMPILER
    conf.env['LINK_CC'] = ARM_CROSS_COMPILER
    conf.load('compiler_c')

def build(bld):
    print("Hello world!")
