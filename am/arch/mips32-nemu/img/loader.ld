SECTIONS {
  . = 0x100000;
  .text : {
    *(.text)
  }
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data)
  }
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss)
    *(.scommon)
  }
  end = .;
  _end = .;
  _heap_start = ALIGN(4096);
  _stack_pointer = 0x8000000;
  _heap_end = _stack_pointer - 4 * 1024 * 1024;
}
