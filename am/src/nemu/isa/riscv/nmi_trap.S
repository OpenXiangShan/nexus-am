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

#define NMI_CONTEXT_SIZE ((32 + 3) * REGBYTES)
#define OFFSET_SP        ( 2 * REGBYTES)
#define OFFSET_CAUSE     (32 * REGBYTES)
#define OFFSET_STATUS    (33 * REGBYTES)
#define OFFSET_EPC       (34 * REGBYTES)

.align 4
.globl __am_asm_nmi_trap
__am_asm_nmi_trap:
  # NMI handler - uses mnscratch (0x740) for scratch space
  csrrw sp, 0x740, sp  # mnscratch
  bnez sp, nmi_save_context
  # trap from M-mode kernel, restore the original sp
  csrrw sp, 0x740, sp  # mnscratch

nmi_save_context:
  addi sp, sp, -NMI_CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrrw t0, 0x740, x0  # mnscratch, t0 = (from user ? usp : 0)
  STORE t0, OFFSET_SP(sp)

  # Read NMI-specific CSRs
  csrr t0, 0x742  # mncause
  csrr t1, 0x744  # mnstatus
  csrr t2, 0x741  # mnepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)

  # Call NMI handler
  mv a0, sp
  jal __am_nmi_handle

  mv sp, a0

  # Restore NMI-specific CSRs
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw 0x744, t1  # mnstatus
  csrw 0x741, t2  # mnepc

  MAP(REGS, POP)

  addi sp, sp, NMI_CONTEXT_SIZE

  csrw 0x740, sp   # mnscratch (ksp)
  LOAD sp, (OFFSET_SP - NMI_CONTEXT_SIZE)(sp) # sp = (from user ? usp : 0)

  bnez sp, nmi_return
  # return to M-mode kernel
  csrrw sp, 0x740, sp  # mnscratch

nmi_return:
  # mnret instruction (encoded directly since toolchain may not support it)
  # mnret = 0x70200073
  .word 0x70200073
