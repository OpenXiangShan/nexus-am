#include <am.h>
#include <riscv32.h>
#include <klib.h>

static uint32_t mul(uint32_t a, uint32_t b, int sign, int hi) {
  if (a == 0x80000000 && b == 0x80000000) {
    // hard code the result for this special case
    const R64 res = {.val = 0x4000000000000000LL};
    return (hi ? res.hi : res.lo);
  }
  else if (b == 0x80000000) {
    // swap the operands
    uint32_t t = a;
    a = b;
    b = t;
  }

  int sign_a = 0, sign_b = 0;
  if (sign) {
    if ((int32_t)a < 0) { sign_a = 1; a = -a; }
    if ((int32_t)b < 0) { sign_b = 1; b = -b; }
  }

  // booth algorithm
  R64 P = {.hi =  0, .lo = b };
  uint32_t choose = (P.lo & 1) << 1;
  int i;
  for (i = 32; i > 0; i --) {
    switch (choose) {
      case 1: P.hi += a; break;
      case 2: P.hi -= a; break;
    }
    choose = P.lo & 0x3;
    P.val >>= 1;
  }

  if (sign_a ^ sign_b) P.val = -P.val;
  return (hi ? P.hi : P.lo);
}

static uint32_t div(uint32_t a, uint32_t b, int sign, int reminder) {
  int sign_a = 0, sign_b = 0;
  if (sign) {
    if ((int32_t)a < 0) { sign_a = 1; a = -a; }
    if ((int32_t)b < 0) { sign_b = 1; b = -b; }
  }

  // non-performing restoring division

  R64 R = {.val = a};
  R.val <<= 1;
  int i;
  for (i = 32; i > 1; i --) {
    if (R.hi >= b) {
      R.hi -= b;
      R.val = (R.val << 1) + 1;
    }
    else {
      R.val <<= 1;
    }
  }

  R.lo <<= 1;
  if (R.hi >= b) {
    R.hi -= b;
    R.lo ++;
  }

  if (sign_a ^ sign_b) R.lo = -R.lo;
  if (sign_a) R.hi = -R.hi;
  return (reminder ? R.hi : R.lo);
}

int __am_illegal_instr(_Context *c) {
  union {
    struct {
      uint32_t opcode :7;
      uint32_t rd     :5;
      uint32_t func   :3;
      uint32_t rs1    :5;
      uint32_t rs2    :5;
      uint32_t func2  :7;
    };
    uint32_t val;
  } instr;
  instr.val = *(uint32_t *)(c->epc);

  if (instr.opcode == 0x33 && instr.func2 == 1) {
    // M extension
    uint32_t rs1 = c->gpr[instr.rs1];
    uint32_t rs2 = c->gpr[instr.rs2];
    switch (instr.func) {
      /* mul   */ case 0: c->gpr[instr.rd] = mul(rs1, rs2, true, false); return true;
      /* mulh  */ case 1: c->gpr[instr.rd] = mul(rs1, rs2, true, true); return true;
      /* div   */ case 4: c->gpr[instr.rd] = div(rs1, rs2, true, false); return true;
      /* divu  */ case 5: c->gpr[instr.rd] = div(rs1, rs2, false, false); return true;
      /* rem   */ case 6: c->gpr[instr.rd] = div(rs1, rs2, true, true); return true;
      /* remu  */ case 7: c->gpr[instr.rd] = div(rs1, rs2, false, true); return true;
    }
  }

  printf("invalid instruction\n");
  _halt(1);
  return false;
}

uint32_t __mulsi3 (uint32_t a, uint32_t b) {
  // mul a0, a0, a1
  asm volatile(".word 0x02b50533");
  return a;
//  return mul(a, b, true, false);
}
uint32_t __divsi3 (uint32_t a, uint32_t b) { return div(a, b, true, false); }
uint32_t __modsi3 (uint32_t a, uint32_t b) { return div(a, b, true, true); }
uint32_t __umodsi3(uint32_t a, uint32_t b) { return div(a, b, false, true); }
uint32_t __udivsi3(uint32_t a, uint32_t b) { return div(a, b, false, false); }
