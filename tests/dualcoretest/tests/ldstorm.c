#include <am.h>
#include <klib.h>

#define N 2048

int load_storm_buffer = 114514;
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

int load_storm(){
  for(int i = 0; i < N; i++){
    asm(
      "ld a0, 0(%0)\n"
      :
      :"r"(&load_storm_buffer)
    );
  }
  return 0;
}

int load_store_storm(){
  load_storm_buffer = 0;
  int load_result = 0;
  for(int i = 0; i < N; i++){
    asm(
      "ld %0, 0(%1)\n"
      "addi %0, %0, 1\n"
      "st %0, 0(%1)\n"
      :"=r"(load_result)
      :"r"(&load_storm_buffer)
    );
  }
  return 0;
}

int main(){
  printf("dual core test start\n");
  int hartid = get_hartid();
  switch(hartid){
    case 0:
      load_storm();
      core_finish_flag[0] = 1;
      asm("fence\n");
      break;
    case 1:
      load_storm();
      core_finish_flag[1] = 1;
      asm("fence\n");
      break;
    default:
      printf("hart 0x%x has nothing to do\n", hartid);
      while(1);
  }
  while(!(core_finish_flag[0] && core_finish_flag[1]));
  success();
  return 0;
}
