.section entry, "ax"
.globl _start
.type _start, @function

#define MSTATUS_FS 0x00006000

.macro set_reg_zero reg_idx
  mv x\reg_idx, zero
.endm

.macro set_freg_zero freg_idx
  fmv.w.x f\freg_idx, zero
.endm

.macro init_regs
  .altmacro
  .set i, 1
  .rept 31
    set_reg_zero %i
    .set i, i+1
  .endr
.endm

.macro init_fregs
  .set i, 0
  .rept 32
    set_freg_zero %i
    .set i, i+1
  .endr
.endm

_start:
  init_regs

  li a0, MSTATUS_FS & (MSTATUS_FS >> 1)
  csrs mstatus, a0
  csrwi fcsr, 0

  init_fregs # init fregs after fp enable

  la t0, _stack_top
  la t1, _stack_pointer
  sub t3, t1, t0
  csrr t4, mhartid
  mul t5, t3, t4
  add sp, t5, t1

  jal _trm_init
