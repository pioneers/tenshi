ARM_CROSS_COMPILER_C = 'arm-none-eabi-gcc'
ARM_CROSS_COMPILER_CXX = 'arm-none-eabi-g++'

from waf_extensions import declare_variants
declare_variants('deb', 'rel', 'debopt', 'relopt')

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('gas')

# Configure fragments used for variants

def common_configure(conf):
    conf.env['CC'] = ARM_CROSS_COMPILER_C
    conf.env['CC_NAME'] = ARM_CROSS_COMPILER_C
    conf.env['COMPILER_CC'] = ARM_CROSS_COMPILER_C
    conf.env['LINK_CC'] = ARM_CROSS_COMPILER_C
    conf.env['CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env['CXX_NAME'] = ARM_CROSS_COMPILER_CXX
    conf.env['COMPILER_CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env['LINK_CXX'] = ARM_CROSS_COMPILER_CXX
    conf.env['AS'] = ARM_CROSS_COMPILER_C
    conf.env.append_value('CFLAGS', [
            '-g',                       # Debug symbols
            '-Wall',                    # All warnings
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-fshort-double',           # sizeof(double) == sizeof(float)
            '-ffunction-sections',      # Each function in individual section (saves space)
            '-fdata-sections',          # Same for data
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
            '-fdata-sections',          # Same for data
            '-std=gnu++98',             # C++ code is gnu++98 standard
        ])
    conf.env.append_value('ASFLAGS', [
            '-g',                       # Debug symbols
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-x', 'assembler-with-cpp', # Compile assembly
        ])
    conf.env.append_value('LINKFLAGS', [
            '-g',                       # Debug symbols
            '-mthumb',                  # Cortex-M4 only supports Thumb
            '-mcpu=cortex-m4',          # Compile for Cortex-M4
            '-mfpu=fpv4-sp-d16',        # Enable FPU opcodes
            '-mfloat-abi=hard',         # Pass arguments via FPU registers
            '-Wl,--gc-sections',        # Discard unused sections
            '-B' + conf.path.abspath(), # For finding .ld file
            '-specs=' + conf.path.abspath() + '/linkspec.specs',    # Linker rules + crt0
        ])
    conf.load('compiler_c')
    conf.load('compiler_cxx')
    conf.load('gas')

def debug_configure(conf):
    conf.env.append_value('CFLAGS', ['-DDEBUG'])
    conf.env.append_value('CXXFLAGS', ['-DDEBUG'])
    conf.env.append_value('ASFLAGS', ['-DDEBUG'])

def release_configure(conf):
    conf.env.append_value('CFLAGS', ['-DRELEASE'])
    conf.env.append_value('CXXFLAGS', ['-DRELEASE'])
    conf.env.append_value('ASFLAGS', ['-DRELEASE'])

def noopt_configure(conf):
    conf.env.append_value('CFLAGS', ['-O0'])
    conf.env.append_value('CXXFLAGS', ['-O0'])
    conf.env.append_value('ASFLAGS', ['-O0'])

def opt_configure(conf):
    conf.env.append_value('CFLAGS', ['-O2'])
    conf.env.append_value('CXXFLAGS', ['-O2'])
    conf.env.append_value('ASFLAGS', ['-O2'])

# Actual configure rules for variants

def configure_deb(conf):
    conf.setenv('deb')
    common_configure(conf)
    debug_configure(conf)
    noopt_configure(conf)

def configure_rel(conf):
    conf.setenv('rel')
    common_configure(conf)
    release_configure(conf)
    noopt_configure(conf)

def configure_debopt(conf):
    conf.setenv('debopt')
    common_configure(conf)
    debug_configure(conf)
    opt_configure(conf)

def configure_relopt(conf):
    conf.setenv('relopt')
    common_configure(conf)
    release_configure(conf)
    opt_configure(conf)

def configure(conf):
    configure_deb(conf)
    configure_rel(conf)
    configure_debopt(conf)
    configure_relopt(conf)

# Build step

def build(bld):
    if not bld.variant:
        print('Building all variants.')
        from waflib.Scripting import run_command
        run_command('build_deb')
        run_command('build_rel')
        run_command('build_debopt')
        run_command('build_relopt')
    else:
        bld.objects(
            source = "stm32f4-crt0.S",
            target = "crt0",
        )

        bld.program (
            source = "src/main.c",
            target = "main",
            use = "crt0",
        )
