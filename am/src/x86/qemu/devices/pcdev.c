#include "../../x86-qemu.h"

static inline uintptr_t port_read(int port, size_t nmemb) {
  switch (nmemb) {
    case 1: return inb(port);
    case 2: return inw(port);
    case 4: return inl(port);
  }
  return 0;
}

static inline void port_write(int port, size_t nmemb, uintptr_t data) {
  switch (nmemb) {
    case 1: return outb(port, data);
    case 2: return outw(port, data);
    case 4: return outl(port, data);
  }
}

size_t pciconf_read(uintptr_t reg, void *buf, size_t size) {
  outl(0xcf8, reg);
  switch (size) {
    case 1: *(uint8_t *) buf = inb(0xcfc + (reg & 3));
    case 2: *(uint16_t *)buf = inw(0xcfc + (reg & 2));
    case 4: *(uint32_t *)buf = inl(0xcfc);
  }
  return size;
}

size_t pciconf_write(uintptr_t reg, void *buf, size_t size) {
  outl(0xcf8, reg);
  switch (size) {
    case 1: outb(0xcfc + (reg & 3), *(uint8_t *) buf);
    case 2: outw(0xcfc + (reg & 2), *(uint16_t *)buf);
    case 4: outl(0xcfc            , *(uint32_t *)buf);
  }
  return size;
}
