MEMORY {
  flash (rxa) : ORIGIN = 0x10000000, LENGTH = 16M
  ram (rwxa) : ORIGIN = 0x80000000, LENGTH = 64M
}

/*
PHDRS {
  ram PT_LOAD;
}
  */

ENTRY(_start)

SECTIONS {
  . = ORIGIN(flash);
  .text : {
    *(entry)
    _mainargs = .;
    *(.text)
  }
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }

  /*. = ORIGIN(ram); */
  .data : {
    *(.data)
  } /*> ram :ram */
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } /*> ram : ram */
  end = .;
  _end = .;

  . = ALIGN(0x1000);
  _stack_pointer = . + - ORIGIN(flash) + ORIGIN(ram) + 0x8000;
  _heap_start = _stack_pointer;
  _heap_end = ORIGIN(ram) + LENGTH(ram);
}
