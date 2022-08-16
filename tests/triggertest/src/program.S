#if __riscv_xlen == 64
# define STORE    sd
# define LOAD     ld
# define REGBYTES 8
#else
# define STORE    sw
# define LOAD     lw
# define REGBYTES 4
#endif

#define STACK_SIZE 4 * REGBYTES

.globl heap
.globl program_0
.globl pc_0

program_0:
  mv t5, a0
  li t0, 1
  li t1, 2
  li t2, 3
  add t3, t0, t1
pc_0:
  add t3, t3, t1
  add t3, t3, t0
  add t3, t3, t1
  STORE t3, 0(t5)
pc_1:
  add t3, t0, t2
  STORE t3, REGBYTES(t5)
  LOAD t4, 0(t5)
  add t3, t4, t2
pc_2:
  LOAD t4, REGBYTES(t5)
  STORE t3, 0(t5)
  ret