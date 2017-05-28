SECTIONS
{
    . = 0x10000000;
    .text : { *(.text) }
    .data : { *(.data) }
    __bss_start = . ;
    .bss : {
	*(.bss)
	*(.scommon)
	*(.sbss)
    }
    _end = . ;
}
