MEMORY {
  bram : ORIGIN = 0x0, LENGTH = 8K
  ddr : ORIGIN = 0x10000000, LENGTH = 128M
}

SECTIONS {
  loader : { *(loader) } > bram
  .text : { *(.text) } > ddr
  .data : { *(.data) } > ddr
  .bss : {
    __bss_start = .;
    *(.bss*)
    *(.sbss)
    *(.scommon)
    _end = .;
  } > ddr
}
