#include "common.h"

#define RAMDISK_START (void *)0x1000
#define RAMDISK_END (void *)0xa0000
#define RAMDISK_SIZE ((RAMDISK_END) - (RAMDISK_START))

/* The kernel is monolithic, therefore we do not need to
 * translate the address `buf' from the user process to
 * a physical one, which is necessary for a microkernel.
 */

/* read `len' bytes starting from `offset' of ramdisk into `buf' */
void ramdisk_read(uint8_t *buf, uint32_t offset, uint32_t len) {
  assert(offset + len < RAMDISK_SIZE);
  memcpy(buf, RAMDISK_START + offset, len);
}

/* write `len' bytes starting from `buf' into the `offset' of ramdisk */
void ramdisk_write(uint8_t *buf, uint32_t offset, uint32_t len) {
  assert(offset + len < RAMDISK_SIZE);
  memcpy(RAMDISK_START + offset, buf, len);
}
