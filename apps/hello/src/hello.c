#include <hello.h>
#include <am.h>

int main() {
  _ioe_init();
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
  _halt(0);
}
