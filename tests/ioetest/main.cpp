#include <am.h>
#include <amdev.h>
#include <klib.h>

_Device *pci;

uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  return pci->read(_DEV_PCICONF_REG(bus, slot, func, offset), 4);
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

void pci_test(_Device *dev) {
  pci = dev;
  for (int i = 0; i < 256; i ++)
    for (int j = 0; j < 32; j ++) {
      check_dev(i, j);
    }
  printk("Get e1000 at %d:%d\n", e1000.bus, e1000.slot);
}

void timer_test(_Device *dev) {
  uint32_t t0 = dev->read(_DEV_TIMER_REG_UPTIME, 4);
  for (int volatile i = 0; i < 1000000; i ++) ;
  uint32_t t1 = dev->read(_DEV_TIMER_REG_UPTIME, 4);
  printk("Loop 10^6 time elapse: %d ms\n", t1 - t0);
}

void ata_test(_Device *dev) {
  while ((dev->read(_DEV_ATA_REG_STATUS, 1) & 0xc0) != 0x40);
  int offset = 0;
  dev->write(_DEV_ATA_REG_NSECT, 1, 1);
  dev->write(_DEV_ATA_REG_SECT, 1, offset);
  dev->write(_DEV_ATA_REG_CYLOW, 1, offset >> 8);
  dev->write(_DEV_ATA_REG_CYHIGH, 1, offset >> 16);
  dev->write(_DEV_ATA_REG_DRIVE, 1, (offset >> 24) | 0xe0);
  dev->write(_DEV_ATA_REG_STATUS, 1, 0x20);
  while ((dev->read(7, 1) & 0xc0) != 0x40);
  uint32_t buf[512 / 4];
  for (int i = 0; i < 512 / 4; i ++) {
    buf[i] = dev->read(_DEV_ATA_REG_DATA, 4);
  }
  for (int i = 0; i < 512; i ++) {
    printf("%02x ", ((char*)buf)[i] & 0xff);
  }
}

int main() {
  for (int n = 1; ; n ++) {
    _Device *dev = _device(n);
    if (!dev) break;
    printf("Device: %s\n", dev->name);
    switch(dev->id) {
      case _DEV_TIMER: timer_test(dev); break;
      case _DEV_PCICONF: pci_test(dev); break;
      case _DEV_ATA0: ata_test(dev); break;
    }
    printf("\n");
  }
  return 0;
}
