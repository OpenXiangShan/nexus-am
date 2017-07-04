SECTIONS {
  . = 0x100000;
  entry : { *(entry) }
  .text : {
    *(.text*)
  }
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data*)
  }
  edata = .;
  _edata = .;
  .bss : {
    *(.bss*)
  }
  end = .;
  _end = .;
  _heap_start = ALIGN(4096);
  _stack_pointer = 0x8000;
  _heap_end = 0x8000000;
}
