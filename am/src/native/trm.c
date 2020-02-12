#include <am.h>
#include <stdio.h>

void __am_platform_dummy();
void __am_exit_platform(int code);

void _trm_init() {
  __am_platform_dummy();
}

void _putc(char ch) {
  fputc(ch, stderr);
}

void _halt(int code) {
  printf("Exit (%d)\n", code);
  __am_exit_platform(code);
  printf("Should not reach here!\n");
  while (1);
}

_Area _heap = {};
