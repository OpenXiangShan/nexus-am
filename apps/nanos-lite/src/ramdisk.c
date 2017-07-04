#include "common.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

/* The kernel is monolithic, therefore we do not need to
 * translate the address `buf' from the user process to
 * a physical one, which is necessary for a microkernel.
 */

/* read `len' bytes starting from `offset' of ramdisk into `buf' */
void ramdisk_read(void *buf, off_t offset, size_t len) {
  if (len == 0) return;

  assert(offset + len <= RAMDISK_SIZE);
  memcpy(buf, &ramdisk_start + offset, len);
}

/* write `len' bytes starting from `buf' into the `offset' of ramdisk */
void ramdisk_write(const void *buf, off_t offset, size_t len) {
  if (len == 0) return;

  assert(offset + len <= RAMDISK_SIZE);
  memcpy(&ramdisk_start + offset, buf, len);
}
