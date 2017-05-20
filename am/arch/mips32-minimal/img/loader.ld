SECTIONS
{
  . = 0x10000000;
  . += 8192;
  _stack_pointer = .;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  _end = .;
}
