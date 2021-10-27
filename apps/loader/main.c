// /*
//  * @Author: your name
//  * @Date: 2021-10-20 20:37:55
//  * @LastEditTime: 2021-10-20 20:37:56
//  * @LastEditors: your name
//  * @Description: In User Settings Edit
//  * @FilePath: /nexus-am/apps/loader/main.c
//  */
// #include <am.h>
// #include <klib.h>
// #include <elf.h>

// extern uint8_t image;

// #define ADDR(x) (uint32_t)(uintptr_t)(x)
// #define ADDRPTR(x) (void *)(uintptr_t)(ADDR(x))
// int main() {

//   Elf64_Ehdr *elf = (void *)&image;
//   Elf64_Phdr *ph, *eph;

//   /* Load each program segment */
//   ph = (void *)elf + elf->e_phoff;
//   eph = ph + elf->e_phnum;
//   for (; ph < eph; ph ++) {
//     if (ph->p_type == PT_LOAD) {
//       memcpy(ADDRPTR(ph->p_vaddr), &image + ADDR(ph->p_offset), ADDR(ph->p_filesz));

//       // now we still do not have any code in icache
//       //if (ph->p_flags & PF_X) {
//       //  asm volatile("fence.i");
//       //  Log("code");
//       //}

//       //printf("zeroing memory region [0x%08x, 0x%08x)\n",
//       //    ADDR(ph->p_vaddr) + ADDR(ph->p_filesz), ADDR(ph->p_vaddr) + ADDR(ph->p_memsz));
//       memset(ADDRPTR(ph->p_vaddr) + ADDR(ph->p_filesz), 0, ADDR(ph->p_memsz) - ADDR(ph->p_filesz));
//     }
//   }

//   /* Check each program segment */
// #if 0
//   int wrong_time = 0;
//   ph = (void *)elf + elf->e_phoff;
//   eph = ph + elf->e_phnum;
//   for (; ph < eph; ph ++) {
//     if (ph->p_type == PT_LOAD) {
//       printf("checking memory region [0x%08x, 0x%08x)\n", ADDR(ph->p_vaddr), ADDR(ph->p_vaddr + ph->p_filesz));
//       int i;
//       for (i = 0; i < ph->p_filesz; i ++) {
//         uint8_t membyte = ((uint8_t *)ph->p_vaddr)[i];
//         uint8_t flashbyte = (&image)[ph->p_offset + i];
//         if (membyte != flashbyte) {
//           wrong_time ++;
//           printf("%8dth wrong byte detect at address 0x%08x, right = %02x, wrong = %02x, diff = %02x\n",
//               wrong_time, ADDR(ph->p_vaddr + i), flashbyte, membyte, flashbyte ^ membyte);
//         }
//       }
//     }
//   }
// #endif

//   void (*f)(void) = (void *)(uintptr_t)elf->e_entry;

//   //int i;
//   //for (i = 0; i < 2; i ++) {
//   //  printf("%02x: instr[%d] = 0x%08x\n", i * 4, i, ((uint32_t *)(void*)f)[i]);
//   //}

//   //printf("jump to 0x%x...\n", ADDR(f));
//   f(); // here we go
//   return 0;
// }

#include "klib.h"

extern uint8_t image;

int a[10] = {};

uint32_t* code = (uint32_t *)0x80000000;

void test(uint8_t *image) {
  //printf("copy code...\n");
  memcpy(code, image, 7776);

#ifdef __ISA_RISCV64__
  asm volatile("fence.i");
#endif

  void (*f)(void) = (void *)code;
  //printf("jump to  code...\n");
  f();
  //printf("ret = %d\n", ret);
}

int main() {
#ifdef __ISA_NATIVE__
# error Can not run with native, since data segment is not executable.
#endif
  for (int i = 0; i < 10; i ++) {
    a[i] = i + 1;
  }

  test(&image);

  return 0;
}
