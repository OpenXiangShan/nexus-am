SECTIONS
{
    . = 0x00000000;
    . = ALGN(4);
    .text : { *(.text) }
    .data : { *(.data) }
    . = 0xf0000000;
    . = ALGN(4);
    __bss_start = . ;
    .bss : {
	*(.bss)
	*(.scommon)
	*(.sbss)
    }
    _end = . ;
}
