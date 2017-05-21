#include <am.h>

void print(const char *);

int main() {
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
  _halt(0);
}
