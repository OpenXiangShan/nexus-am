SECTIONS
{
    . = 0x00000000;
    .text : {
        _ftext = . ;
        *(.text)
	*(.rodata)
	*(.rodate1)
	*(.reginfo)
	*(.init)
	*(.stub)
	*(.gnu.warning)
    } =0
    _etext = .;
    PROVIDE (etext = .);
    .fini : { *(.fini) } =0
    .data : {
	_fdata = . ;
	*(.data)
    }
    .data1 : { *(.data1) }
    . = 0xf0000000;
    __bss_start = .;
    _fbss = . ;
    .sbss : { *(.sbss) *(.scommon) }
    .bss :
    {
	*(.dynbss)
	*(.bss)
	*(COMMON)
    }
    . = ALIGN(16);
    __bss_end = 0xf2000000;
    . = 0xf2000004;
    _heap_start = .;
    _heap_end = 0xf6000000;
}
