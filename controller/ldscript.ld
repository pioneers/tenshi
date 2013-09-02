OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	flash		: ORIGIN = 0x08000000, LENGTH = 1M
	ccmram		: ORIGIN = 0x10000000, LENGTH = 64K
	sram		: ORIGIN = 0x20000000, LENGTH = 128K
}

__sp_main		= ORIGIN(sram) + LENGTH(sram);

SECTIONS
{
	.vectors :
	{
		__text_start = . ;
		KEEP (*(.vectors))
		. = ALIGN(4);
	} >flash = 0xff

	.init :
	{
		KEEP (*(.init))
		. = ALIGN(4);
	} >flash = 0xff

	.text :
	{
		*(.text)
		*(.text.*)
		*(.stub)
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);
	} >flash = 0xff

	.fini :
	{
		KEEP (*(.fini))
		. = ALIGN(4);
	} >flash = 0

	__text_end = . ;



	.rodata :
	{
		*(.rodata)
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);
	} >flash = 0xff

	ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >flash
	__exidx_start = .;
	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >flash
	__exidx_end = .;



	. = ALIGN(32 / 8);
	PROVIDE (__preinit_array_start = .);
	.preinit_array :
	{
		KEEP (*(.preinit_array))
	} >flash = 0xff
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array :
	{
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
	} >flash = 0xff
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     :
	{
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
	} >flash = 0xff
	PROVIDE (__fini_array_end = .);

	.ctors :
	{
		/*	gcc uses crtbegin.o to find the start of the constructors, so
			we make sure it is first.  Because this is a wildcard, it
			doesn't matter if the user does not actually link against
			crtbegin.o; the linker won't look for a file to match a
			wildcard.  The wildcard also means that it doesn't matter which
			directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);
	} >flash = 0

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);
	} >flash = 0


	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);
	} >flash = 0

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);
	} >flash = 0

	.jcr : 
	{
		KEEP (*(.jcr))
		. = ALIGN(4);
	} >flash = 0



	__ccmram_lma = . ;
	.ccmram : AT(__ccmram_lma)
	{
		__ccmram_start = ABSOLUTE(.);
		*(.ccmram)
		*(.ccmram.*)
		. = ALIGN(4);
	} >ccmram = 0xff
	__ccmram_end = ABSOLUTE(.);



	__data_lma = __ccmram_lma + SIZEOF(.ccmram) ;
	.data : AT(__data_lma)
	{
		__data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
	} >sram = 0xff
	__data_end = ABSOLUTE(.);
	__rom_end = __data_lma + SIZEOF(.data);



	.sbss (NOLOAD):
	{
		__sbss_start = ABSOLUTE(.);
		__sbss_start__ = ABSOLUTE(.);
		*(.sbss)
		. = ALIGN(4);
		__sbss_end = ABSOLUTE(.);
	} >sram

	.bss (NOLOAD): 
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = ABSOLUTE(.) ;
		__bss_end__ = ABSOLUTE(.) ;
		__end__ = ABSOLUTE(.) ;
	} >sram



	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000 : { _stack = .; *(.stack) }
	/* DWARF 3 */
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	.debug_ranges   0 : { *(.debug_ranges) }
	/* DWARF Extension.  */
	.debug_macro    0 : { *(.debug_macro) }
	.ARM.attributes 0 : { KEEP (*(.ARM.attributes)) KEEP (*(.gnu.attributes)) }
	.note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
	/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
	/* These must appear regardless of  .  */
}