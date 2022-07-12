#include <am.h>
#include <riscv.h>
#include <klib.h>

extern char _heap_start;
extern char _pmem_end;
extern char sdata, edata, _sidata;
extern char _bss_start,  _bss_end;
int main(const char *args);
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);

_Area _heap = {
  .start = &_heap_start,
  .end = &_pmem_end,
};

void _putc(char ch) {
  __am_uartlite_putchar(ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));

  // should not reach here during simulation
  printf("Exit with code = %d\n", code);

  // should not reach here on FPGA
  while (1);
}

void _copy_data() {
  char* flash_data = &_sidata;
  char* ram_data   = &sdata;

  while(ram_data != &edata){
    *ram_data = *flash_data;
    flash_data++;
    ram_data++;
  }
}

void _init_bss(){
  char* iteration = &_bss_start;
  
  while (iteration != &_bss_end)
  {
    *iteration = 0;
    iteration++;
  }
  
}

void _trm_init() {
  _copy_data();
  _init_bss();
  __am_init_uartlite();
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}
