pmem_base = 0x80000000;

MEMORY {
  ram (rwxa) : ORIGIN = 0x80000000, LENGTH = 128M
}

INCLUDE "section.ld"
