SECTIONS
{
    . = 0x00000000;
    .text : { *(.text) } =0
    .data : { *(.data) }
    . = 0xf0000000;
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = . ;
    _heap_start = 0xf2000000;
}
