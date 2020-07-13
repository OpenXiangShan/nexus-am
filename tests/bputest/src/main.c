#include <am.h>
#include <klib.h>
#define LINES 128
#define COLLUMNS 128

uint64_t array[LINES][COLLUMNS];

void iteration() {
  for(uint64_t i = 0; i < LINES; i++) {
    for(uint64_t j = 0; j < COLLUMNS; j++) {
      array[i][j] = i + j;
    }
  }
}

int main() {
  printf("Test start!\n");
  iteration();
  printf("Test end!\n");
  return 0;
}
