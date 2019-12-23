MEMORY {
  flash (rxa) : ORIGIN = 0x40000000, LENGTH = 16M
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

  . = ORIGIN(ram);
  _stack_pointer = . + 0x2000000;
  _heap_start = _stack_pointer;
  _heap_end = ORIGIN(ram) + LENGTH(ram);
}
