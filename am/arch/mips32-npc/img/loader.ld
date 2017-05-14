SECTIONS
{
    . = 0x00000000;
    .text : {
        _ftext = . ;
        *(.text)
	*(.rodata)
	*(.rodata1)
	*(.reginfo)
	*(.init)
	*(.stub)
	*(.gnu.warning)
    } =0
    .fini : { *(.fini) } =0
    .data : {
	_fdata = . ;
	*(.data)
    }
    .data1 : { *(.data1) }
    . = 0xf0000000;
    __bss_start = .;
    .sbss : { *(.sbss) *(.scommon) }
    .bss :
    {
	*(.dynbss)
	*(.bss)
	*(COMMON)
    }
    __bss_end = . ;
    . = 0xf2000000;
    _heap_start = . ;
}
