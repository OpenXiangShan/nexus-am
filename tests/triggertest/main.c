#include <am.h>
#include <klib.h>
#include "trigger.h"
#include "encoding.h"
#include "debug_defines.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

void identify_triggers(void);
int frontend_trigger_test(void);
void trigger_write(void);
void inst_addr_test(void);
void inst_data_test(void);
void load_addr_test(void);
void store_addr_test(void);
void inst_chain_test(void);

int main() {
  // write mtvec
  write_csr(mtvec, &&trap_entry);

  // trigger_write();
  // identify_triggers();
  // inst_addr_test();
  // inst_data_test();
  store_addr_test();
  load_addr_test();
  inst_chain_test();

  write_csr(mepc, &&finish_test - 4);
trap_entry:
  printf("trapped with cause %lx at addr %lx\n", read_csr(mcause), read_csr(mepc));
  goto *(read_csr(mepc) + 4);
finish_test:
  return 0;
}

void identify_triggers(void) {
  printf("Examining triggers...\n");
  uint64_t tselect, tinfo, tdata1;
  for (int i = 0; i < 11; i ++) {
    tselect = read_csr(tselect);
    tinfo = read_csr(0x7a4); // tinfo
    tdata1 = read_csr(tdata1);  
    // tselect
    printf("tselect is %x; ", tselect);
    // tinfo
    printf("tinfo is %lx; ", tinfo);
    // tdata 1
    printf("mcontrol is %lx\n", tdata1);
    tselect ++;
    write_csr(tselect, i);
  }
  printf("Trigger examination completed successfully!\n");
}

void trigger_write(void) {
  // Trigger write test
  uint64_t data_to_write = 0x80000000037005F; // enable this trigger timing is 1
  // data_to_write.tdata1.mcontrol.type = 2;
  for (int i = 0; i < 10; i ++) {
    write_csr(tselect, i);
    write_csr(tdata1, data_to_write);
  }
}

void inst_addr_test(void) {
  printf("Begin inst addr test\n");
  int a = 0;
  write_csr(tselect, 0);
  uint64_t mcontrol_to_write = 0;

  MCONTROL_WRITE(mcontrol_to_write, EXECUTE, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZEHI, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZELO, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, M, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 1) ;
  // printf("mcontrol_to_write is %lx\n", mcontrol_to_write);
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, &&add_2);
  a += 1;
add_2:
  a += 2;
  printf("Added 2\n");
  a += 3;
}

void inst_chain_test(void) {
  printf("Begin inst addr test\n");
  int a = 0;
  write_csr(tselect, 0);
  uint64_t mcontrol_to_write = 0;
  MCONTROL_WRITE(mcontrol_to_write, EXECUTE, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZEHI, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZELO, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 2) ;
  MCONTROL_WRITE(mcontrol_to_write, CHAIN, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, M, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 1) ;
  // printf("mcontrol_to_write is %lx\n", mcontrol_to_write);
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, &&add_1);
  write_csr(tselect, 1);
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 3) ;
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, &&add_3);

add_1:
  a += 1;
  a += 2;
add_3:
  a += 3;
}


void inst_data_test(void) {
  printf("Begin inst data test\n");
  int a = 0;
  write_csr(tselect, 0);
  uint64_t mcontrol_to_write = 0;
  MCONTROL_WRITE(mcontrol_to_write, EXECUTE, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZEHI, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZELO, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, SELECT, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, M, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 1) ;
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, 0x0057879b); // add a5 a5 5
  a += 4;
  printf("Added 4\n");
  a += 5;
  printf("Added 5\n");
  a += 6;
  printf("Added 6\n");
  MCONTROL_WRITE(mcontrol_to_write, M, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 0) ;
}

void load_addr_test(void) {
  int a = 0;
  write_csr(tselect, 5);
  printf("Begin load addr test\n");
  uint64_t mcontrol_to_write = 0;
  MCONTROL_WRITE(mcontrol_to_write, LOAD, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZEHI, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZELO, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, M, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 1) ;
  uint64_t n;
  asm volatile ("mv %0, s0" : "=r"(n));
  n -= 20;
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, n);
  a += 1;
  // printf("End load addr test\n");
}

void store_addr_test(void){
  int a = 0;
  write_csr(tselect, 3);
  printf("Begin store addr test\n");
  uint64_t mcontrol_to_write = 0;
  MCONTROL_WRITE(mcontrol_to_write, STORE, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZEHI, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, SIZELO, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, MATCH, 0) ;
  MCONTROL_WRITE(mcontrol_to_write, M, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, S, 1) ;
  MCONTROL_WRITE(mcontrol_to_write, U, 1) ;
  uint64_t n;
  asm volatile ("mv %0, s0" : "=r"(n));
  n -= 20;
  write_csr(tdata1, mcontrol_to_write);
  write_csr(tdata2, n);
  a += 1;
  // printf("End store addr test\n");
}
#pragma GCC pop_options