#include <amtest.h>

void hello() {
  for (int i = 0; i < 10; i ++) {
    putstr("Hello, AM World @ " __ISA__ "\n");
  }
}

void hello_n(int n) {
  for (int i = 0; i < n; i ++) {
    putstr("Hello, AM World @ " __ISA__ "\n");
  }
}
