pmem_base = 0x800a0000;

MEMORY {
  ram (rwxa) : ORIGIN = 0x800a0000, LENGTH = 128M
}

INCLUDE "section.ld"
