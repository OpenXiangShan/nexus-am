MEMORY {
  mem : ORIGIN = 0x00000000, LENGTH = 128M
}

SECTIONS {
  . = ORIGIN(mem);
  .text : {
    *(.text)
  } > mem
  .data : {
    *(.data)
  } > mem
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss)
    *(.scommon)
    _bss_end = .;
  } > mem
  _heap_start = ALIGN(4096);
  _stack_pointer = ORIGIN(mem) + LENGTH(mem);
  _heap_end = _stack_pointer - 4 * 1024 * 1024;
}
