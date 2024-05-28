# Assembly Trap Entry with save and restore context

#define REGBYTES  8
#define STORE     sd
#define LOAD      ld

# marco for save context
.macro SAVE_CONTEXT
  # We just storage CONTEXT into current stack.
  add   sp, sp, -(32 * REGBYTES)
  
  STORE x1, 1*REGBYTES(sp)
  # Skip x2/sp register
  STORE x3, 3*REGBYTES(sp)
  STORE x4, 4*REGBYTES(sp)
  STORE x5, 5*REGBYTES(sp)
  STORE x6, 6*REGBYTES(sp)
  STORE x7, 7*REGBYTES(sp)
  STORE x8, 8*REGBYTES(sp)
  STORE x9, 9*REGBYTES(sp)
  STORE x10, 10*REGBYTES(sp)
  STORE x11, 11*REGBYTES(sp)
  STORE x12, 12*REGBYTES(sp)
  STORE x13, 13*REGBYTES(sp)
  STORE x14, 14*REGBYTES(sp)
  STORE x15, 15*REGBYTES(sp)
  STORE x16, 16*REGBYTES(sp)
  STORE x17, 17*REGBYTES(sp)
  STORE x18, 18*REGBYTES(sp)
  STORE x19, 19*REGBYTES(sp)
  STORE x20, 20*REGBYTES(sp)
  STORE x21, 21*REGBYTES(sp)
  STORE x22, 22*REGBYTES(sp)
  STORE x23, 23*REGBYTES(sp)
  STORE x24, 24*REGBYTES(sp)
  STORE x25, 25*REGBYTES(sp)
  STORE x26, 26*REGBYTES(sp)
  STORE x27, 27*REGBYTES(sp)
  STORE x28, 28*REGBYTES(sp)
  STORE x29, 29*REGBYTES(sp)
  STORE x30, 30*REGBYTES(sp)
  STORE x31, 31*REGBYTES(sp)
.endm

# marco for restore context
.macro RESTORE_CONTEXT
  # We just load CONTEXT from current stack.
  LOAD  x1, 1*REGBYTES(sp)
  # Skip x2/sp register
  LOAD  x3, 3*REGBYTES(sp)
  LOAD  x4, 4*REGBYTES(sp)
  LOAD  x5, 5*REGBYTES(sp)
  LOAD  x6, 6*REGBYTES(sp)
  LOAD  x7, 7*REGBYTES(sp)
  LOAD  x8, 8*REGBYTES(sp)
  LOAD  x9, 9*REGBYTES(sp)
  LOAD  x10, 10*REGBYTES(sp)
  LOAD  x11, 11*REGBYTES(sp)
  LOAD  x12, 12*REGBYTES(sp)
  LOAD  x13, 13*REGBYTES(sp)
  LOAD  x14, 14*REGBYTES(sp)
  LOAD  x15, 15*REGBYTES(sp)
  LOAD  x16, 16*REGBYTES(sp)
  LOAD  x17, 17*REGBYTES(sp)
  LOAD  x18, 18*REGBYTES(sp)
  LOAD  x19, 19*REGBYTES(sp)
  LOAD  x20, 20*REGBYTES(sp)
  LOAD  x21, 21*REGBYTES(sp)
  LOAD  x22, 22*REGBYTES(sp)
  LOAD  x23, 23*REGBYTES(sp)
  LOAD  x24, 24*REGBYTES(sp)
  LOAD  x25, 25*REGBYTES(sp)
  LOAD  x26, 26*REGBYTES(sp)
  LOAD  x27, 27*REGBYTES(sp)
  LOAD  x28, 28*REGBYTES(sp)
  LOAD  x29, 29*REGBYTES(sp)
  LOAD  x30, 30*REGBYTES(sp)
  LOAD  x31, 31*REGBYTES(sp)

  add   sp, sp, (32 * REGBYTES)
.endm

# entry to trap into M-mode
.text
.align 4
.global m_trap_entry
m_trap_entry:
  SAVE_CONTEXT
  jal m_exception_handler
  RESTORE_CONTEXT
  mret
