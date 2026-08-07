ENTRY(_start)

SECTIONS {
  . = ORIGIN(ram);
  .text : {
    *(entry)
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

  /* Eight 128 KiB stack/TLS regions, one for each supported hart. */
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x100000;
  _stack_pointer = .;

  /* thread-local data segment */
  .tdata :
  {
    _tdata_begin = .;
    *(.tdata .tdata.*)
    _tdata_end = .;
  }
  .tbss :
  {
    _tbss_begin = .;
    *(.tbss .tbss.*)
    _tbss_end = .;
  }

  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
  _pmem_start = pmem_base;
  _pmem_end = _pmem_start + LENGTH(ram);
}
