SECTIONS {
  . = 0x100000;
  entry : { *(entry) }
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : {
    _bss_start = .;
    *(.bss*)
    _bss_end = .;
  }
  _heap_start = ALIGN(4096);
  _stack_pointer = 128M;
  _heap_end = _stack_pointer - 4 * 1024 * 1024;
}
