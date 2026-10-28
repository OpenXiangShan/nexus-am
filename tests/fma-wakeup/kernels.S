/* All routines are leaf functions and use only caller-saved registers.
 * a0: Result output, a1: input when applicable; Result has a 16-byte stride.
 * No stack frame or startup/exit implementation is needed. */
.text
.option norvc

.macro FUNCTION name
  .balign 4
  .global \name
  .type \name, @function
\name:
.endm
.macro FINISH name
  ret
  .size \name, .-\name
.endm
.macro REGION_BEGIN name
  .global \name\()_begin
\name\()_begin:
.endm
.macro REGION_END name
  .global \name\()_end
\name\()_end:
.endm
.macro LOAD_D reg, bits
  li t0, \bits
  fmv.d.x \reg, t0
.endm
.macro LOAD_S reg, bits
  li t0, \bits
  fmv.w.x \reg, t0
.endm
.macro RECORD reg
  fsd \reg, 0(a0)
  csrr t0, fflags
  sw t0, 8(a0)
  addi a0, a0, 16
.endm
/* Memory ROIs avoid CSR reads because those can serialize speculation.
 * The final accrued flags are copied to the final record after the ROI. */
.macro RECORD_VALUE reg
  fsd \reg, 0(a0)
  sw zero, 8(a0)
  addi a0, a0, 16
.endm
.macro CLEAR
  csrw fcsr, zero
.endm

FUNCTION functional_cases
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fmsub.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fnmsub.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fnmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fmadd.s ft0, ft0, ft1, ft2
  RECORD ft0
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fmadd.s ft1, ft0, ft1, ft2
  RECORD ft1
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fmadd.s ft2, ft0, ft1, ft2
  RECORD ft2
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fadd.s ft0, ft2, ft2
  fmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fadd.s ft1, ft0, ft2
  fmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fadd.s ft2, ft4, ft2
  fmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x40000000
  LOAD_S ft1, 0x40400000
  LOAD_S ft2, 0x3f800000
  LOAD_S ft4, 0x00000000
  fadd.s ft0, ft2, ft2
  fmadd.s ft3, ft0, ft0, ft0
  RECORD ft3
  CLEAR
  LOAD_S ft0, 0x3f800001
  LOAD_S ft1, 0x3f7ffffe
  LOAD_S ft2, 0xbf800000
  fmadd.s ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fmsub.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fnmsub.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fnmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fmadd.d ft0, ft0, ft1, ft2
  RECORD ft0
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fmadd.d ft1, ft0, ft1, ft2
  RECORD ft1
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fmadd.d ft2, ft0, ft1, ft2
  RECORD ft2
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fadd.d ft0, ft2, ft2
  fmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fadd.d ft1, ft0, ft2
  fmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fadd.d ft2, ft4, ft2
  fmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  LOAD_D ft4, 0
  fadd.d ft0, ft2, ft2
  fmadd.d ft3, ft0, ft0, ft0
  RECORD ft3
  CLEAR
  LOAD_D ft0, 0x3ff0000000000001
  LOAD_D ft1, 0x3feffffffffffffe
  LOAD_D ft2, 0xbff0000000000000
  fmadd.d ft3, ft0, ft1, ft2
  RECORD ft3
FINISH functional_cases

FUNCTION add_chain
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x3ff0000000000000
  REGION_BEGIN add_chain
  .rept 64
    fadd.d ft0, ft0, ft1
  .endr
  REGION_END add_chain
  RECORD ft0
FINISH add_chain

FUNCTION mul_chain
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x4000000000000000
  REGION_BEGIN mul_chain
  .rept 64
    fmul.d ft0, ft0, ft1
  .endr
  REGION_END mul_chain
  RECORD ft0
FINISH mul_chain

FUNCTION fma_multiplicand_chain
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x3ff0000000000000
  REGION_BEGIN fma_multiplicand_chain
  .rept 64
    fmadd.d ft0, ft0, ft1, ft1
  .endr
  REGION_END fma_multiplicand_chain
  RECORD ft0
FINISH fma_multiplicand_chain

FUNCTION fma_rs2_chain
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x3ff0000000000000
  REGION_BEGIN fma_rs2_chain
  .rept 64
    fmadd.d ft0, ft1, ft0, ft1
  .endr
  REGION_END fma_rs2_chain
  RECORD ft0
FINISH fma_rs2_chain

FUNCTION fma_accumulator_chain
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x3ff0000000000000
  REGION_BEGIN fma_accumulator_chain
  .rept 64
    fmadd.d ft0, ft1, ft1, ft0
  .endr
  REGION_END fma_accumulator_chain
  RECORD ft0
FINISH fma_accumulator_chain

FUNCTION mul_to_fma_acc
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x3ff0000000000000
  REGION_BEGIN mul_to_fma_acc
  .rept 32
    fmul.d ft0, ft0, ft1
    fmadd.d ft0, ft2, ft2, ft0
  .endr
  REGION_END mul_to_fma_acc
  RECORD ft0
FINISH mul_to_fma_acc

FUNCTION duplicate_source_wakeup
  CLEAR
  LOAD_D ft0, 0x3ff0000000000000
  LOAD_D ft1, 0x4000000000000000
  REGION_BEGIN duplicate_source_wakeup
  .rept 24
    fmul.d ft0, ft0, ft1
    fmadd.d ft3, ft0, ft0, ft0
  .endr
  REGION_END duplicate_source_wakeup
  RECORD ft3
FINISH duplicate_source_wakeup

FUNCTION late_arrival
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  REGION_BEGIN late_arrival
  fmul.d ft3, ft0, ft1
  .rept 16
    nop
  .endr
  fmadd.d ft4, ft3, ft0, ft2
  REGION_END late_arrival
  RECORD ft4
FINISH late_arrival

FUNCTION fma_to_store
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  REGION_BEGIN fma_to_store
  fmadd.d ft3, ft0, ft1, ft2
  fsd ft3, 0(a0)
  REGION_END fma_to_store
  csrr t0, fflags
  sw t0, 8(a0)
FINISH fma_to_store

FUNCTION fma_to_convert
  CLEAR
  LOAD_D ft0, 0x4000000000000000
  LOAD_D ft1, 0x4008000000000000
  LOAD_D ft2, 0x3ff0000000000000
  REGION_BEGIN fma_to_convert
  fmadd.d ft3, ft0, ft1, ft2
  fcvt.l.d t1, ft3, rtz
  REGION_END fma_to_convert
  sd t1, 0(a0)
  csrr t0, fflags
  sw t0, 8(a0)
FINISH fma_to_convert

/* Each iteration outputs its changing value, exposing stale operand data. */
FUNCTION load_hot_addend
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  li t1, 256
  li t2, 8
  REGION_BEGIN load_hot_addend
.Lload_hot_addend_loop:
  fld ft0, 0(a1)
  fmadd.d ft3, ft1, ft2, ft0
  RECORD_VALUE ft3
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lload_hot_addend_loop
  REGION_END load_hot_addend
  csrr t0, fflags
  sw t0, -8(a0)
FINISH load_hot_addend

FUNCTION load_hot_multiplicand
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  li t1, 256
  li t2, 8
  REGION_BEGIN load_hot_multiplicand
.Lload_hot_multiplicand_loop:
  fld ft0, 0(a1)
  fmadd.d ft3, ft0, ft2, ft1
  RECORD_VALUE ft3
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lload_hot_multiplicand_loop
  REGION_END load_hot_multiplicand
  csrr t0, fflags
  sw t0, -8(a0)
FINISH load_hot_multiplicand

FUNCTION load_cold_addend
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  li t1, 256
  li t2, 4096
  REGION_BEGIN load_cold_addend
.Lload_cold_addend_loop:
  fld ft0, 0(a1)
  .global load_cold_addend_fma
load_cold_addend_fma:
  fmadd.d ft3, ft1, ft2, ft0
  RECORD_VALUE ft3
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lload_cold_addend_loop
  REGION_END load_cold_addend
  csrr t0, fflags
  sw t0, -8(a0)
FINISH load_cold_addend

FUNCTION load_cold_multiplicand
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  li t1, 256
  li t2, 4096
  REGION_BEGIN load_cold_multiplicand
.Lload_cold_multiplicand_loop:
  fld ft0, 0(a1)
  .global load_cold_multiplicand_fma
load_cold_multiplicand_fma:
  fmadd.d ft3, ft0, ft2, ft1
  RECORD_VALUE ft3
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lload_cold_multiplicand_loop
  REGION_END load_cold_multiplicand
  csrr t0, fflags
  sw t0, -8(a0)
FINISH load_cold_multiplicand

FUNCTION fma_cascade
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  LOAD_D ft5, 0x3ff0000000000000
  li t1, 256
  li t2, 4096
  REGION_BEGIN fma_cascade
.Lcascade_loop:
  fld ft0, 0(a1)
  .global cascade_fma_one
cascade_fma_one:
  fmadd.d ft3, ft1, ft2, ft0
  .global cascade_fma_two
cascade_fma_two:
  fmadd.d ft4, ft3, ft1, ft5
  RECORD_VALUE ft4
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lcascade_loop
  REGION_END fma_cascade
  csrr t0, fflags
  sw t0, -8(a0)
FINISH fma_cascade

FUNCTION fma_pressure
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  LOAD_D fa0, 0x3ff0000000000000
  fmv.d ft4, fa0
  fmv.d ft5, fa0
  fmv.d ft6, fa0
  fmv.d ft7, fa0
  fmv.d ft8, fa0
  fmv.d ft9, fa0
  fmv.d ft10, fa0
  fmv.d ft11, fa0
  li t1, 256
  li t2, 4096
  REGION_BEGIN fma_pressure
.Lpressure_loop:
  fld ft0, 0(a1)
  .global fma_pressure_target
fma_pressure_target:
  fmadd.d ft3, ft1, ft2, ft0
  fmadd.d ft4, fa0, fa0, ft4
  fmadd.d ft5, fa0, fa0, ft5
  fmadd.d ft6, fa0, fa0, ft6
  fmadd.d ft7, fa0, fa0, ft7
  fmadd.d ft8, fa0, fa0, ft8
  fmadd.d ft9, fa0, fa0, ft9
  fmadd.d ft10, fa0, fa0, ft10
  fmadd.d ft11, fa0, fa0, ft11
  RECORD_VALUE ft3
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lpressure_loop
  REGION_END fma_pressure
  csrr t0, fflags
  sw t0, -8(a0)
  RECORD ft4
  RECORD ft5
  RECORD ft6
  RECORD ft7
  RECORD ft8
  RECORD ft9
  RECORD ft10
  RECORD ft11
FINISH fma_pressure

/* First 255 outcomes train fall-through with valid 1*0+0. The final
 * page supplies infinity and a taken condition: speculated inf*0+0 must
 * neither commit its destination nor leak NV into architectural fflags.
 * The committed result also consumes ft3, whose preserved value must be zero.
 * Four dependent integer multiplies delay resolution after the load. */
FUNCTION branch_squash
  CLEAR
  LOAD_D ft1, 0x4000000000000000
  LOAD_D ft2, 0x4008000000000000
  LOAD_D fa0, 0x3ff0000000000000
  fmv.d.x ft5, zero
  fmv.d.x ft3, zero
  li t1, 256
  li t2, 4096
  REGION_BEGIN branch_squash
.Lsquash_loop:
  csrw fflags, zero
  fld ft0, 8(a1)
  ld t3, 0(a1)
  mul t3, t3, t3
  mul t3, t3, t3
  mul t3, t3, t3
  mul t3, t3, t3
  bnez t3, .Lsquash_taken
  .global wrongpath_fma
wrongpath_fma:
  fmadd.d ft3, ft0, ft5, ft5
.Lsquash_taken:
  fmadd.d ft4, ft1, ft2, fa0
  fadd.d ft4, ft4, ft3
  RECORD ft4
  add a1, a1, t2
  addi t1, t1, -1
  bnez t1, .Lsquash_loop
  REGION_END branch_squash
FINISH branch_squash

/* Integer expressions form exact IEEE binary64 encodings for 1..256.
 * Each cold page has one datum and is accessed only inside its own ROI. */
.section .rodata
.macro INPUT name, stride
  .balign 4096
  .global \name
  .type \name, @object
\name:
  .set .Lnode, 1
  .set .Lexponent, 0
  .rept 256
    .if .Lnode == (2 << .Lexponent)
      .set .Lexponent, .Lexponent + 1
    .endif
    .quad ((1023 + .Lexponent) << 52) | ((.Lnode - (1 << .Lexponent)) << (52 - .Lexponent))
    .zero \stride - 8
    .set .Lnode, .Lnode + 1
  .endr
  .size \name, .-\name
.endm
INPUT hot_addend, 8
INPUT hot_multiplicand, 8
INPUT cold_addend, 4096
INPUT cold_multiplicand, 4096
INPUT cascade_input, 4096
INPUT pressure_input, 4096

.balign 4096
.global squash_input
.type squash_input, @object
squash_input:
.set .Lnode, 1
.rept 256
  .if .Lnode == 256
    .quad 1, 0x7ff0000000000000
  .else
    .quad 0, 0x3ff0000000000000
  .endif
  .zero 4080
  .set .Lnode, .Lnode + 1
.endr
.size squash_input, .-squash_input

.section .note.GNU-stack,"",@progbits
