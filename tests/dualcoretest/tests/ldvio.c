#include <am.h>
#include <klib.h>

#define N 2048

int load_test_buffer = 0;
int core_finish_flag[16] = {};

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
  int hartid = -1;
  asm(
    "csrr a0, mhartid\n"
    "sd a0, %0\n"
    :"=m"(hartid)
    :
    :"a0"
  );
  printf("get hart id: %x\n");
  if(hartid < 0){
    printf("invalid hartid\n");
    failure();
  }
  return hartid;
}

int hart0_fast_addr_load(int* p_last_load_result){
  int load_result = 0;
  asm(
    "ld a0, 0(%0)\n"
    "sd a0, 0(%1)\n"
    :
    :"r"(&load_test_buffer), "r"(&load_result)
    :"a0"
  );
  assert(*p_last_load_result <= load_result);
  *p_last_load_result = load_result;
  return 0;
}

int hart0_slow_addr_load(int* p_last_load_result){
  int load_result = 0;
  asm(
    "fsqrt.d f0, f1\n"
    "fcvt.l.d a1, f0\n"
    "srli a1, a1, 24\n"
    "srli a1, a1, 24\n"
    "srli a1, a1, 24\n"
    "add a0, a1, %0\n"
    "ld a0, 0(%0)\n"
    "sd a0, 0(%1)\n"
    :
    :"r"(&load_test_buffer), "r"(&load_result)
    :"a0", "a1"
  );
  assert(*p_last_load_result <= load_result);
  *p_last_load_result = load_result;
  return 0;
}

int hart0_workload(){
  int last_load_result = -1;
  for(int i = 0; i < N; i++){
    if(rand() % 2){
      hart0_fast_addr_load(&last_load_result);
    }else{
      hart0_slow_addr_load(&last_load_result);
    }
  }
  return 0;
}

int hart1_workload(){
  int i = 0;
  while(1){
    // keep writing to load_test_buffer
    asm(
      "sd %1, 0(%0)\n"
      "fence\n"
      :
      :"r"(&load_test_buffer), "r"(i)
    );
    i++;
  };
  return 0;
}

int main(){
  printf("dual core test start\n");
  int hartid = get_hartid();
  switch(hartid){
    case 0:
      hart0_workload();
      core_finish_flag[0] = 1;
      asm("fence\n");
      break;
    case 1:
      hart1_workload();
      core_finish_flag[1] = 1;
      asm("fence\n");
      break;
    default:
      printf("hart 0x%x has nothing to do\n", hartid);
      while(1);
  }
  while(!(core_finish_flag[0]));
  success();
  return 0;
}
