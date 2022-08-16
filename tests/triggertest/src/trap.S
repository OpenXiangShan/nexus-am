#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#if __riscv_xlen == 64
# define STORE    sd
# define LOAD     ld
# define REGBYTES 8
#else
# define STORE    sw
# define LOAD     lw
# define REGBYTES 4
#endif

#define PUSH(n) STORE concat(x, n), (n * REGBYTES)(sp);
#define POP(n)  LOAD  concat(x, n), (n * REGBYTES)(sp);

#define CONTEXT_SIZE ((32 + 3) * REGBYTES)
#define OFFSET_SP     ( 2 * REGBYTES)
#define OFFSET_CAUSE  (32 * REGBYTES)
#define OFFSET_STATUS (33 * REGBYTES)
#define OFFSET_EPC    (34 * REGBYTES)
.align 4
.globl trap_entry

trap_entry:
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  STORE t0, OFFSET_SP(sp)

  csrr a0, mcause
  csrr a1, mstatus
  csrr a2, mepc

  STORE a0, OFFSET_CAUSE(sp)
  STORE a1, OFFSET_STATUS(sp)
  STORE a2, OFFSET_EPC(sp)

  mv a0, sp
  jal handle_trap

#  mv sp, a0

  LOAD a1, OFFSET_STATUS(sp)
  LOAD a2, OFFSET_EPC(sp)
  csrw mstatus, a1
  csrw mepc, a2

  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE
  mret
