ARM_CROSS_COMPILER_C = 'arm-none-eabi-gcc'
ARM_CROSS_COMPILER_CXX = 'arm-none-eabi-g++'

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')

def configure(conf):
    conf.env['CC'] = ARM_CROSS_COMPILER_C
    conf.env['CC_NAME'] = ARM_CROSS_COMPILER_C
    conf.env['COMPILER_CC'] = ARM_CROSS_COMPILER_C
    conf.env['LINK_CC'] = ARM_CROSS_COMPILER_C
    conf.env['CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env['CXX_NAME'] = ARM_CROSS_COMPILER_CXX
    conf.env['COMPILER_CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env['LINK_CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env.append_value('CFLAGS', [
            '-g',                       # Debug symbols
            '-Wall',                    # All warnings
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-fshort-double',           # sizeof(double) == sizeof(float)
            '-ffunction-sections',      # Each function in individual section (saves space)
            '-fdata-sections,'          # Same for data
            '-std=gnu99',               # C code is gnu99 standard
        ])
    conf.env.append_value('CXXFLAGS', [
            '-g',                       # Debug symbols
            '-Wall',                    # All warnings
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-fshort-double',           # sizeof(double) == sizeof(float)
            '-ffunction-sections',      # Each function in individual section (saves space)
            '-fdata-sections,'          # Same for data
            '-std=gnu++98',             # C++ code is gnu++98 standard
        ])
    conf.env.append_value('LINKFLAGS', [
            '-g',                       # Debug symbols
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-Wl,--gc-sections'         # Discard unused sections
        ])
    conf.load('compiler_c')
    conf.load('compiler_cxx')

def build(bld):
    bld.program (
        source = "src/main.c",
        target = "main",
    )
