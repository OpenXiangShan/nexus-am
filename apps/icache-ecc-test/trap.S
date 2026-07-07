#if __riscv_xlen == 64
# define STORE    sd
# define LOAD     ld
# define REGBYTES 8
#else
# define STORE    sw
# define LOAD     lw
# define REGBYTES 4
#endif

#define CONTEXT_SIZE (5 * REGBYTES)
#define OFF_RA       (0 * REGBYTES)
#define OFF_T0       (1 * REGBYTES)
#define OFF_T1       (2 * REGBYTES)
#define OFF_T2       (3 * REGBYTES)
#define OFF_T3       (4 * REGBYTES)

  .section .text
  .align 2
  .globl icache_ecc_trap_entry
icache_ecc_trap_entry:
  addi sp, sp, -CONTEXT_SIZE
  STORE ra, OFF_RA(sp)
  STORE t0, OFF_T0(sp)
  STORE t1, OFF_T1(sp)
  STORE t2, OFF_T2(sp)
  STORE t3, OFF_T3(sp)

  csrr a0, mcause
  csrr a1, mtval
  csrr a2, mstatus
  csrr a3, mepc
  call exception_handler

  csrw mepc, a0

  LOAD ra, OFF_RA(sp)
  LOAD t0, OFF_T0(sp)
  LOAD t1, OFF_T1(sp)
  LOAD t2, OFF_T2(sp)
  LOAD t3, OFF_T3(sp)
  addi sp, sp, CONTEXT_SIZE
  mret
