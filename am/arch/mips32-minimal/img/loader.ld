MEMORY {
  bram : ORIGIN = 0, LENGTH = 8K
}

SECTIONS {
  .text : {
    *(.text)
    etext = .;
    _etext = .;
  } > bram
  .data : {
    *(.data)
    edata = .;
    _edata = .;
  } > bram
  .bss : {
    *(.bss*)
    *(.sbss)
    *(.scommon)
    end = .;
    _end = .;
  } > bram
  stack : {
    . += 1024;
    _stack_pointer = ALIGN(4);
  } > bram
} 
