#include <klib.h>

int strlen(const char *s) {
  int sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}
