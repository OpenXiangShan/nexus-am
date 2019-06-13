#include <amunit.h>

void hellworld() {
  printf("Hello, AM World!\n");
}

void print_msg() {
}

int main(const char *args) {
  switch (args[0]) {
    CASE('1', "hello", hellworld)
    CASE('2', "hello", hellworld, IOE, CTE(NULL))
    default:
      printf("Usage: make run mainargs=*\n");
  }
  return 0;
}
