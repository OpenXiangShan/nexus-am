pmem_base = 0x80000000;

MEMORY {
  ram (rwxa) : ORIGIN = 0x80100000, LENGTH = 1024M
}

INCLUDE "section.ld"
