#output dir
BUILD ?= build
LIBSDIR = libs
INCLUDESDIR = includes
BINDIR = programs

DEVICE=atmega32u4
CPUFREQ=16000000

ifneq ($(BUILD),$(notdir $(CURDIR)))
#when make in run in toplevel directory

#export SILENT = 
export SILENT = @

export EXTRA_LIBS = -lm

export TOPDIR = $(CURDIR)

#hack for LUFA headers
export INCLUDES = $(TOPDIR)/$(LIBSDIR)

export CFLAGS += -gdwarf-2 -Wall -Os -mmcu=$(DEVICE) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -std=gnu99 -ffunction-sections -fdata-sections -DF_CPU=$(CPUFREQ) -DUSE_LUFA_CONFIG_HEADER
export ASFLAGS += -gdwarf-2 -Wall -mmcu=$(DEVICE)
export LDFLAGS += -gdwarf-2 -Wall -mmcu=$(DEVICE) -Wl,--gc-sections

.PHONY:	all clean $(BUILD)

all:	$(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD)

else
#when make is run in the build directory

# $(eval $(call build-library,name))
# this tries to auto search for all the needed source files
define build-library
$(LIBSDIR):	$(LIBSDIR)/$1.a
$(LIBSDIR)/$1.a:	$(LIBSDIR)/$1/$1.a
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)cp $$< $$@
$(LIBSDIR)/$1/$1.a:	$$(subst .asm,.o,$$(subst .c,.o,$$(subst .S,.o,$$(subst .s,.o,$$(subst $$(TOPDIR)/,,$$(shell find $$(TOPDIR)/$$(LIBSDIR)/$1 -type f -name '*.asm' -o -name '*.c' -o -name '*.S' -o -name '*.s'))))))
	@echo $$(notdir $$@)
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)rm -f $$@
	$$(SILENT)$$(AR) cr $$@ $$^
-include $$(subst .asm,.P,$$(subst .c,.P,$$(subst .S,.P,$$(subst $$(TOPDIR)/,,$$(shell find $$(TOPDIR)/$$(LIBSDIR)/$1 -type f -name '*.asm' -o -name '*.c' -o -name '*.S' -o -name '*.s')))))
endef

# $(eval $(call build-program,name,deps))
# this tries to auto search for all the needed source files
define build-program
$1/$1.elf : LDFLAGS += -Wl,-Map=$(BINDIR)/$1.map
$(BINDIR):	$(BINDIR)/$1.hex $(BINDIR)/$1.lst
$(BINDIR)/$1.hex:	$1/$1.hex
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)cp $$< $$@
$(BINDIR)/$1.lst:	$1/$1.lst
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)cp $$< $$@
$1/$1.elf:	$$(subst .asm,.o,$$(subst .c,.o,$$(subst .S,.o,$$(subst .s,.o,$$(subst $$(TOPDIR)/,,$$(shell find $$(TOPDIR)/$1 -type f -name '*.asm' -o -name '*.c' -o -name '*.S' -o -name '*.s'))))))
$1/$1.lst:	$1/$1.elf
$1/$1.elf:	$2
-include $$(subst .asm,.P,$$(subst .c,.P,$$(subst .S,.P,$$(subst $$(TOPDIR)/,,$$(shell find $$(TOPDIR)/$1 -type f -name '*.asm' -o -name '*.c' -o -name '*.S' -o -name '*.s')))))
endef

# $(eval $(call copy-file,path)
# this copies a file at relative path "path" to the build directory
define copy-file
$(BINDIR):	$(BINDIR)/$(notdir $1)
$(BINDIR)/$(notdir $1):	$1
	@echo $$(notdir $$@)
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)cp $$< $$@
endef

# $(eval $(call copy-exe,path)
# this copies a file at relative path "path" to the build directory and make it executable
# it will auto-strip .sh
define copy-exe
$(BINDIR):	$(subst .sh,,$(BINDIR)/$(notdir $1))
$(BINDIR)/$(subst .sh,,$(notdir $1)):	$1
	@echo $$(notdir $$@)
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)cp $$< $$@
	$$(SILENT)chmod +x $$@
endef

# $(eval $(call touch-file,path)
# this creates a file at build/"path"
define touch-file
all:	$1
$1:
	@echo $$(notdir $$@)
	$$(SILENT)[ -d $$(WHERE) ] || mkdir -p $$(WHERE)
	$$(SILENT)touch $$@
endef



.PHONY:	all $(LIBSDIR) $(INCLUDESDIR) $(BINDIR)

all:	$(LIBSDIR) $(INCLUDESDIR) $(BINDIR)

$(INCLUDESDIR):
$(LIBSDIR):
$(BINDIR):

$(BINDIR):	$(LIBSDIR) $(INCLUDESDIR)

$(eval $(call build-program,firmware,libs/avrfixedmath.a libs/LUFA.a))
$(eval $(call build-library,avrfixedmath))
$(eval $(call build-library,LUFA))

#special logic for gitrevision.c
firmware/firmware.elf: firmware/gitrevision.o
firmware/gitrevision.c: $(TOPDIR)/firmware/gitrevision.c.templ
	@echo $(notdir $@)
	$(SILENT)$(TOPDIR)/git-version.sh $< $@
#force rebuild
.PHONY: firmware/gitrevision.c

include $(TOPDIR)/rules.mk

endif
