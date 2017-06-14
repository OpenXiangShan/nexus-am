MEMORY {
  mem : ORIGIN = 0x00000000, LENGTH = 128M
}

SECTIONS {
  . = ORIGIN(mem);
  .text : {
    *(.text)
    etext = .;
    _etext = .;
  } > mem
  .data : {
    *(.data)
    edata = .;
    _data = .;
  } > mem
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss)
    *(.scommon)
    end = .;
    _end = .;
  } > mem
  _heap_start = ALIGN(4096);
  _stack_pointer = ORIGIN(mem) + LENGTH(mem);
  _heap_end = _stack_pointer - 4 * 1024 * 1024;
}
