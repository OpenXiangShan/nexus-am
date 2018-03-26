#include <am.h>
#include <amdev.h>
#include <klib.h>

_Device *pci;

uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  uint32_t res;
  pci->read(_DEVREG_PCICONF(bus, slot, func, offset), &res, 4);
  return res;
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
  _UptimeReg uptime;
  dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  uint32_t t0 = uptime.lo;
  for (int volatile i = 0; i < 10000000; i ++) ;
  dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  uint32_t t1 = uptime.lo;
  printk("Loop 10^7 time elapse: %d ms\n", t1 - t0);
}

static uint8_t readb(_Device *dev, uint32_t reg) {
  uint8_t res;
  dev->read(reg, &res, 1);
  return res;
}

static uint32_t readl(_Device *dev, uint32_t reg) {
  uint32_t res;
  dev->read(reg, &res, 4);
  return res;
}

static void writeb(_Device *dev, uint32_t reg, uint8_t res) {
  dev->write(reg, &res, 1);
}

void ata_test(_Device *dev) {
  while ((readb(dev, _DEVREG_ATA_STATUS) & 0xc0) != 0x40);
  int offset = 0;
  writeb(dev, _DEVREG_ATA_NSECT, 1);
  writeb(dev, _DEVREG_ATA_SECT, offset);
  writeb(dev, _DEVREG_ATA_CYLOW, offset >> 8);
  writeb(dev, _DEVREG_ATA_CYHIGH, offset >> 16);
  writeb(dev, _DEVREG_ATA_DRIVE, (offset >> 24) | 0xe0);
  writeb(dev, _DEVREG_ATA_STATUS, 0x20);
  while ((readb(dev, _DEVREG_ATA_STATUS) & 0xc0) != 0x40);
  uint32_t buf[512 / 4];
  for (int i = 0; i < 512 / 4; i ++) {
    buf[i] = readl(dev, _DEVREG_ATA_DATA);
  }
  for (int i = 0; i < 512; i ++) {
    printf("%02x ", ((char*)buf)[i] & 0xff);
  }
}

int main() {
  _ioe_init();
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
