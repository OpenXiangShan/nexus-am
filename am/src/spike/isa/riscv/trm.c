#include <nemu.h>

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;
# define TOHOST_CMD(dev, cmd, payload) \
  (((uint64_t)(dev) << 56) | ((uint64_t)(cmd) << 48) | (uint64_t)(payload))
#define FROMHOST_DEV(fromhost_value) ((uint64_t)(fromhost_value) >> 56)
#define FROMHOST_CMD(fromhost_value) ((uint64_t)(fromhost_value) << 8 >> 56)
#define FROMHOST_DATA(fromhost_value) ((uint64_t)(fromhost_value) << 16 >> 16)
volatile int htif_console_buf;

static void __check_fromhost()
{
  uint64_t fh = fromhost;
  if (!fh)
    return;
  fromhost = 0;

  // this should be from the console
  // assert(FROMHOST_DEV(fh) == 1);
  switch (FROMHOST_CMD(fh)) {
    case 0:
      htif_console_buf = 1 + (uint8_t)FROMHOST_DATA(fh);
      break;
    case 1:
      break;
    default:
      // assert(0);
      ;
  }
}

static void __set_tohost(uintptr_t dev, uintptr_t cmd, uintptr_t data)
{
  while (tohost)   // tohost need to be 0
    __check_fromhost() // fromhost need to be 0
    ;
  tohost = TOHOST_CMD(dev, cmd, data);
}

void _putc(char ch)
{
  __set_tohost(1, 1, ch);
}

void _halt(int code) {
  __set_tohost(0, 0, (code << 1) | 1);

  while (1);
}
