#include <am.h>
#include <klib.h>

// naive dual core test

// use it with care, IT IS NOT A NORMAL 2 CORE PROGRAM
// ... but it just works

#define N 1000
#define HART_CTRL_RESET_REG_BASE 0x39001000

volatile int load_test_buffer = 0;
volatile int core_init_flag[16] = {0};
volatile int core_finish_flag[16] = {0};
long last_load_result = -1;

void success() {
  printf("test passed.\n");
  asm("li a0, 0\n");
  asm(".word 0x0000006b\n");
}

void failure() {
  printf("test failed.\n");
  asm("li a0, 1\n");
  asm(".word 0x0000006b\n");
}

int get_hartid(){
  long hartid = -1;
  asm(
    "csrr a0, mhartid\n"
    "sd a0, %0\n"
    :"=m"(hartid)
    :
    :"a0"
  );
  // printf("get hart id: %lx\n", hartid);
  if(hartid < 0){
    // printf("invalid hartid\n");
    failure();
  }
  return hartid;
}

void hart0_workload(){
  // printf("hart0_workload started\n");
  for(int i = 0; i < N; i++){
    asm(
      "ld a0, 0(%0)\n"
      :
      :"r"(&load_test_buffer)
    );
  }
  success();
}

void hart1_workload(){
  // no func call, every thing is done within regs
  // printf("hart1_workload started\n");
  for(int i = 0; i < N; i++){
    asm(
      "ld a0, 0(%0)\n"
      :
      :"r"(&load_test_buffer)
    );
  }
  success();
}

int main(){
  // printf("core is running\n");
  int hartid = get_hartid();
  switch(hartid){
    case 0:
      // write hart_ctrl_reset_reg
      *(long*)((long)HART_CTRL_RESET_REG_BASE + 1 * 8) = 0;
      // waiting for hart 1 to update core_init_flag[1]
      while(!core_init_flag[1]){};
      // hart 1 has started up. run real workload
      hart0_workload();
      assert(0);
      break;
    case 1:
      core_init_flag[hartid] = 1;
      asm("fence\n");
      hart1_workload();
      assert(0);
      break;
    default:
      printf("hart 0x%x has nothing to do\n", hartid);
      while(1);
  }
  assert(0);
  return 0;
}
