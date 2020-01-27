#include "x86_64-qemu.h"

int _ioe_init() { return 0; }
size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) { return 0; }
size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) { return 0; }
