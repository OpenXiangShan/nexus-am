#include <klib.h>

//t
size_t strlen(const char *s) {
  size_t sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}
