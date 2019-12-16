#include <am.h>
#include <klib.h>
#include <elf.h>

extern uint8_t image;

#define ADDR(x) (uint32_t)(uintptr_t)(x)
int main() {
  printf("Hello world! Build time: %s %s\n", __DATE__, __TIME__);

  Elf64_Ehdr *elf = (void *)&image;
  Elf64_Phdr *ph, *eph;

  assert(*(uint32_t *)&image == 0x464c457f);

  /* Load each program segment */
  ph = (void *)elf + elf->e_phoff;
  eph = ph + elf->e_phnum;
  for (; ph < eph; ph ++) {
    if (ph->p_type == PT_LOAD) {
      printf("loading to memory region [0x%08x, 0x%08x)\n", ADDR(ph->p_vaddr), ADDR(ph->p_vaddr + ph->p_filesz));
      memcpy((void *)ph->p_vaddr, &image + ph->p_offset, ph->p_filesz);

      // now we still do not have any code in icache
      //if (ph->p_flags & PF_X) {
      //  asm volatile("fence.i");
      //  Log("code");
      //}

      printf("zeroing memory region [0x%08x, 0x%08x)\n",
          ADDR(ph->p_vaddr + ph->p_filesz), ADDR(ph->p_vaddr + ph->p_memsz));
      memset((void *)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }
  }

  void (*f)(void) = (void *)(uintptr_t)elf->e_entry;

  int i;
  for (i = 0; i < 10; i ++) {
    printf("instr[%d] = 0x%08x\n", i, ((uint32_t *)(void*)f)[i]);
  }

  for (i = 0; i < 10; i ++) {
    printf("do_reset[%d] = 0x%08x\n", i, ((uint32_t *)((void*)f + 0x200))[i]);
  }

  printf("jump to 0x%x...\n", ADDR(f));
  f(); // here we go
  return 0;
}
