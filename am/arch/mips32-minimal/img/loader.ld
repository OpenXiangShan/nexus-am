MEMORY {
  bram : ORIGIN = 0, LENGTH = 8K
}

SECTIONS {
  .text : {
    *(.text);
  } > bram
  .data : {
    *(.data);
  } > bram
  .bss : {
    *(.bss);
    _end = .;
  } > bram
  stack : {
    . += 1024;
    _stack_pointer = ALIGN(4);
  } > bram
} 
