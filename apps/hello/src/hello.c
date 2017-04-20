#include <hello.h>
#include <am.h>

int main() {
  _trm_init();
  _ioe_init();
  print("Hello World!\n");
  _halt(0);
}
