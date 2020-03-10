pmem_base = 0x0;

MEMORY {
  ram (rwxa) : ORIGIN = 0x0, LENGTH = 128M
}

INCLUDE "section.ld"
