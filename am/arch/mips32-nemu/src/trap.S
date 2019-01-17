
#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)

#define regs(X) \
      X( 1) X( 2) X( 3) X( 4) X( 5) X( 6) X( 7) X( 8) X( 9) \
X(10) X(11) X(12) X(13) X(14) X(15) X(16) X(17) X(18) X(19) \
X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) X(28)       \
X(30) X(31)

#define CONTEXT_SIZE ((31 + 6) * 4)
#define OFFSET_SP     (29 * 4)
#define OFFSET_LO     (32 * 4)
#define OFFSET_HI     (33 * 4)
#define OFFSET_CAUSE  (34 * 4)
#define OFFSET_STATUS (35 * 4)
#define OFFSET_EPC    (36 * 4)

#define CP0_STATUS 12
#define CP0_CAUSE  13
#define CP0_EPC    14

#define push(n) sw $n, (n * 4)($sp);
#define pop(n)  lw $n, (n * 4)($sp);

.set noat
.globl asm_trap
asm_trap:
  move $k0, $sp
  addiu $sp, $sp, -CONTEXT_SIZE

  regs(push)
  sw $k0, OFFSET_SP($sp)
  mflo $t0
  mfhi $t1
  mfc0 $t2, $CP0_CAUSE
  mfc0 $t3, $CP0_STATUS
  mfc0 $t4, $CP0_EPC
  sw $t0, OFFSET_LO($sp)
  sw $t1, OFFSET_HI($sp)
  sw $t2, OFFSET_CAUSE($sp)
  sw $t3, OFFSET_STATUS($sp)
  sw $t4, OFFSET_EPC($sp)

  move $a0, $sp
  jal irq_handle

  lw $t0, OFFSET_LO($sp)
  lw $t1, OFFSET_HI($sp)
  lw $t2, OFFSET_CAUSE($sp)
  lw $t3, OFFSET_STATUS($sp)
  lw $t4, OFFSET_EPC($sp)
  mtlo $t0
  mthi $t1
  mtc0 $t2, $CP0_CAUSE
  mtc0 $t3, $CP0_STATUS
  mtc0 $t4, $CP0_EPC

  regs(pop)

  eret
