ENTRY(_start)

SECTIONS {
  . = ORIGIN(ram);
  .text : {
    *(entry)
    . = ALIGN(0x1000);
    _mainargs = .;
    . = . + 0x1000;
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
    *(.sbss*)
    *(.scommon)
  }
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
  _heap_end = ORIGIN(ram) + LENGTH(ram);
}
