#include <am.h>
#include <klib.h>
#include <trigger.h>
#include <encoding.h>
#include <debug_defines.h>

void handle_trap(uintptr_t cause, uintptr_t mstatus, uintptr_t mepc);
void init_trap(void);
extern uint32_t pc_0;
extern uint32_t pc_1;
extern uint32_t pc_2;

void identify_triggers(void);
void inst_addr_test(void);
void program_0(uintptr_t *);

TRIGGER_WRITE(pc, EXECUTE, 0)
TRIGGER_WRITE(inst, EXECUTE, 1)
TRIGGER_WRITE(ld, LOAD, 0)
TRIGGER_WRITE(st, STORE, 0)

uintptr_t *heap;

int main() {
  // identify_triggers();
  init_trap();
  heap = malloc(2 * sizeof(uintptr_t));
  inst_addr_test();
  // inst_data_test();
  // store_addr_test();
  // load_addr_test();
  // inst_chain_test();
}

void identify_triggers(void) {
  printf("Examining triggers...\n");
  uint64_t tselect, tinfo, tdata1;
  for (unsigned int i = 0; i < 11; i ++) {
    write_csr(tselect, i);
    tselect = read_csr(tselect);
    tinfo = read_csr(tinfo);
    tdata1 = read_csr(tdata1);
    printf("tselect is %x; ", tselect);
    printf("tinfo is %lx; ", tinfo);
    printf("mcontrol is %lx\n", tdata1);
    tselect ++;
  }
  printf("Trigger examination completed successfully!\n");
}

void inst_addr_test(void){
  pc_trigger(0, (uintptr_t) &pc_0, 0, 0);
  program_0(heap);
  _halt(0);
}