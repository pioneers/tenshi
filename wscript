def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.env.cc = 'arm-none-eabi-gcc'
    #conf.options.check_c_compiler = './tools/arm-toolchain/bin/arm-none-eabi-gcc'
    conf.load('compiler_c')

def build(bld):
    print("Hello world!")
