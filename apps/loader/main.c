#include <am.h>
#include <klib.h>

extern uint8_t image;
extern uint8_t image_end;
#define IMG_SIZE (&image_end - &image)

int main() {
  printf("Hello world! Build time: %s %s\n", __DATE__, __TIME__);
  void (*f)(void) = (void *)(uintptr_t)0x80000000;
  printf("copying image to sdram 0x80000000, size = %d bytes\n", IMG_SIZE);
  memcpy(f, &image, IMG_SIZE);
  printf("jump to 0x80000000...\n");
  f(); // here we go
  return 0;
}
