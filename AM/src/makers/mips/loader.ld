SECTIONS
{
    . = 0x00000000;
    .text : AT(0x00100000) { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    stack_top = .;
}
