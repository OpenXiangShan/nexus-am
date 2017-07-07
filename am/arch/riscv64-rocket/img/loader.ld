SECTIONS {
  . = 0x80000000;
  entry : { *(entry) }
  .text : {
    *(.text*)
  }
  .rodata : {
    *(.rodata*)
  }
  etext = .;
  _etext = .;
  .data : {
    *(.data*)
  }
  edata = .;
  _edata = .;
  .bss : {
    *(.bss*)
  }
  .sbss : {
    *(.sbss*)
  }
  end = .;
  _end = .;
}
