#include <am.h>
#include <klib.h>

_Device *pci;

uint32_t addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  return (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | offset;
}


uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  return pci->read(addr(bus, slot, func, offset), 4);
}

struct pcidev {
  uint8_t bus, slot, func, offset;
} e1000;

void check_dev(uint8_t bus, uint8_t slot) {
  uint32_t info = pci_conf_read(bus, slot, 0, 0);
  uint16_t id = info >> 16, vendor = info & 0xffff;
  if (vendor != 0xffff) {
    printf("Get a device on %d:%d, id %x vendor %x\n", bus, slot, id, vendor);

    if (id == 0x100e && vendor == 0x8086) {
      e1000 = {bus, slot, 0, 0};
    }
  }
}

int main() {
  for (_Device *dev = _devices; dev->id != 0; dev ++) {
    printk("Detected device: %s\n", dev->name);

    if (dev->id == 0x10001) {
      pci = dev;
      for (int i = 0; i < 256; i ++)
        for (int j = 0; j < 32; j ++) {
          check_dev(i, j);
        }
      printk("Get e1000 at %d:%d\n", e1000.bus, e1000.slot);
    }
    if (dev->id == 0x10002) {
      continue;
      while ((dev->read(7, 1) & 0xc0) != 0x40);
      int offset = 0;
      dev->write(2, 1, 1);
      dev->write(3, 1, offset);
      dev->write(4, 1, offset >> 8);
      dev->write(5, 1, offset >> 16);
      dev->write(6, 1, (offset >> 24) | 0xe0);
      dev->write(7, 1, 0x20);
      while ((dev->read(7, 1) & 0xc0) != 0x40);
      uint32_t buf[512 / 4];
      for (int i = 0; i < 512 / 4; i ++) {
        buf[i] = dev->read(0, 4);
      }
      for (int i = 0; i < 512; i ++) {
        printf("%02x ", ((char*)buf)[i] & 0xff);
      }
    }
  }
  return 0;
}
