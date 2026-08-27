.section .text.vector_fill_bytes, "ax", @progbits
.align 2

# void vector_fill_bytes(uint8_t *dst, size_t length)
#
# Initialize the large performance-test inputs without a multi-million
# instruction scalar loop.  The fill is outside all measured regions.
.globl vector_fill_bytes
.type vector_fill_bytes, @function
vector_fill_bytes:
  beqz a1, .Lfill_done
  li t1, 65
.Lfill_loop:
  vsetvli t0, a1, e8, m8, ta, ma
  vmv.v.x v8, t1
  vse8.v v8, (a0)
  add a0, a0, t0
  sub a1, a1, t0
  bnez a1, .Lfill_loop
.Lfill_done:
  ret
.size vector_fill_bytes, .-vector_fill_bytes

# char *vector_strcpy(char *dst, const char *src)
#
# This is the strcpy example from the RISC-V Vector extension manual.  The
# fault-only-first load lets the loop advance by the number of bytes actually
# fetched, while vmsif.m builds the store mask through the first NUL byte.
.section .text.vector_strcpy, "ax", @progbits
.align 2
.globl vector_strcpy
.type vector_strcpy, @function
vector_strcpy:
  mv a2, a0                 # Keep the destination pointer for the stores.
  li t0, -1                 # Infinite AVL: select VLMAX bytes each iteration.

.Lstrcpy_loop:
  vsetvli zero, t0, e8, m8, ta, ma
  vle8ff.v v8, (a1)
  csrr t1, vl
  vmseq.vi v1, v8, 0
  vfirst.m a3, v1
  add a1, a1, t1
  vmsif.m v0, v1
  vse8.v v8, (a2), v0.t
  add a2, a2, t1
  bltz a3, .Lstrcpy_loop
  ret
.size vector_strcpy, .-vector_strcpy

# size_t vector_strlen(const char *s)
.section .text.vector_strlen, "ax", @progbits
.align 2
.globl vector_strlen
.type vector_strlen, @function
vector_strlen:
  mv a2, a0                 # Beginning of the string.
  li t0, -1

.Lstrlen_loop:
  vsetvli zero, t0, e8, m8, ta, ma
  vle8ff.v v8, (a0)
  csrr t1, vl
  vmseq.vi v1, v8, 0
  vfirst.m a1, v1
  bltz a1, .Lstrlen_advance
  add a0, a0, a1
  sub a0, a0, a2
  ret

.Lstrlen_advance:
  add a0, a0, t1
  j .Lstrlen_loop
.size vector_strlen, .-vector_strlen

# int vector_strcmp(const char *s1, const char *s2)
#
# Compare one vector chunk at a time.  The event mask contains both unequal
# bytes and NUL bytes; scalar loads at the first event retain strcmp's unsigned
# byte ordering.  Fault-only-first loads also make the common chunk size the
# smaller of the two chunks when a source crosses a page boundary.
.section .text.vector_strcmp, "ax", @progbits
.align 2
.globl vector_strcmp
.type vector_strcmp, @function
vector_strcmp:
  li t0, -1

.Lstrcmp_loop:
  vsetvli zero, t0, e8, m8, ta, ma
  vle8ff.v v8, (a0)
  csrr t1, vl
  vsetvli zero, t1, e8, m8, ta, ma
  vle8ff.v v16, (a1)
  csrr t1, vl
  beqz t1, .Lstrcmp_scalar

  vmsne.vv v0, v8, v16
  vmseq.vi v1, v8, 0
  vmseq.vi v2, v16, 0
  vmor.mm v1, v1, v2
  vmor.mm v0, v0, v1
  vfirst.m a3, v0
  bltz a3, .Lstrcmp_advance

  add t2, a0, a3
  add t3, a1, a3
  lbu t4, 0(t2)
  lbu t5, 0(t3)
  sub a0, t4, t5
  ret

.Lstrcmp_advance:
  add a0, a0, t1
  add a1, a1, t1
  j .Lstrcmp_loop

# A zero-length fault-only-first chunk is not expected for valid strings, but
# handling it avoids an infinite loop if a caller presents an unusual mapping.
.Lstrcmp_scalar:
  lbu t4, 0(a0)
  lbu t5, 0(a1)
  sub a0, t4, t5
  ret
.size vector_strcmp, .-vector_strcmp

# char *vector_strchr(const char *s, int c)
#
# Search for c or the terminating NUL.  Returning the NUL pointer when c == 0
# matches the C strchr contract; return NULL when c is absent.
.section .text.vector_strchr, "ax", @progbits
.align 2
.globl vector_strchr
.type vector_strchr, @function
vector_strchr:
  mv a2, a0                 # Preserve the current chunk base.
  andi a1, a1, 255
  li t0, -1

.Lstrchr_loop:
  vsetvli zero, t0, e8, m8, ta, ma
  vle8ff.v v8, (a0)
  csrr t1, vl
  vmseq.vx v1, v8, a1
  vmseq.vi v2, v8, 0
  vmor.mm v1, v1, v2
  vfirst.m a3, v1
  bltz a3, .Lstrchr_advance

  add a0, a0, a3
  lbu t2, 0(a0)
  beq t2, a1, .Lstrchr_found
  li a0, 0
  ret

.Lstrchr_advance:
  add a0, a0, t1
  j .Lstrchr_loop

.Lstrchr_found:
  ret
.size vector_strchr, .-vector_strchr
