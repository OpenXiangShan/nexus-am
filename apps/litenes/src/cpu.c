#include <cpu.h>
#include <memory.h>
#include <ppu.h>
#include <klib.h>

//#define STATISTIC

static CPU_STATE cpu;
static unsigned long cpu_cycles;  // Total CPU Cycles Since Power Up (wraps)

static int cycle_table[256] = {
/*0x00*/ 7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
/*0x10*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x20*/ 6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
/*0x30*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x40*/ 6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
/*0x50*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x60*/ 6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
/*0x70*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x80*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0x90*/ 2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
/*0xA0*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0xB0*/ 2,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,
/*0xC0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xD0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0xE0*/ 2,6,3,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xF0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
};

// CPU Adressing Modes
#define cpu_address_implied(exec) { exec(true); }

#define cpu_address_immediate(exec) { \
  uint32_t op_value = instr_fetch(PCreg); \
  PCreg++; \
  exec(true); \
}

#define cpu_address_zero_page(exec) { \
  uint32_t op_address = instr_fetch(PCreg); \
  uint32_t op_value = cpu_ram_read(op_address); \
  PCreg++; \
  exec(true); \
}

#define cpu_address_zero_page_x(exec) { \
  uint32_t op_address = (instr_fetch(PCreg) + Xreg) & 0xFF; \
  uint32_t op_value = cpu_ram_read(op_address); \
  PCreg++; \
  exec(true); \
}

#define cpu_address_zero_page_y(exec) { \
  uint32_t op_address = (instr_fetch(PCreg) + Yreg) & 0xFF; \
  uint32_t op_value = cpu_ram_read(op_address); \
  PCreg++; \
  exec(true); \
}

#define cpu_address_absolute(exec) { \
  uint32_t op_address = instr_fetchw(PCreg); \
  uint32_t op_value = memory_readb(op_address); \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_absolute_x(exec) { \
  uint32_t op_address = (instr_fetchw(PCreg) + Xreg) & 0xffff; \
  uint32_t op_value = memory_readb(op_address); \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_absolute_y(exec) { \
  uint32_t op_address = (instr_fetchw(PCreg) + Yreg) & 0xFFFF; \
  uint32_t op_value = memory_readb(op_address); \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_relative(exec) { \
  exec(false); \
}

#define cpu_address_indirect(exec) { \
  uint32_t arg_addr = instr_fetchw(PCreg); \
  uint32_t op_address; \
 \
  /* The famous 6502 bug when instead of reading from $C0FF/$C100 it reads from $C0FF/$C000 */ \
  if ((arg_addr & 0xFF) == 0xFF) { \
    op_address = (memory_readb(arg_addr & 0xFF00) << 8) | memory_readb(arg_addr); \
  } \
  else { \
    op_address = memory_readw(arg_addr); \
  } \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_indirect_x(exec) { \
  uint32_t arg_addr = (instr_fetch(PCreg) + Xreg) & 0xFF; \
  assert(0); \
  uint32_t op_address = cpu_ram_readw(arg_addr); \
  uint32_t op_value = memory_readb(op_address); \
  PCreg++; \
  exec(false); \
}

#define cpu_address_indirect_y(exec) { \
  uint32_t arg_addr = instr_fetch(PCreg); \
  uint32_t temp = cpu_ram_readw(arg_addr); \
  uint32_t op_address = (temp + (Yreg & 0xff)) & 0xFFFF; \
  uint32_t op_value = memory_readb(op_address); \
  PCreg++; \
  exec(false); \
}

// addressing which does not load the op_value

#define cpu_address_zero_page_notload(exec) { \
  uint32_t op_address = instr_fetch(PCreg); \
  PCreg++; \
  exec(true); \
}

#define cpu_address_zero_page_x_notload(exec) { \
  uint32_t op_address = (instr_fetch(PCreg) + Xreg) & 0xFF; \
  PCreg++; \
  exec(true); \
}

#define cpu_address_zero_page_y_notload(exec) { \
  uint32_t op_address = (instr_fetch(PCreg) + Yreg) & 0xFF; \
  PCreg++; \
  exec(true); \
}

#define cpu_address_absolute_notload(exec) { \
  uint32_t op_address = instr_fetchw(PCreg); \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_absolute_jmp(exec) { \
  uint32_t op_address = instr_fetchw(PCreg); \
  if (op_address == PCreg - 1) { \
    /* spin to wait for interrupt, just end the cpu run directly */ \
    cycles = 0; \
  } \
  exec(false); \
}

#define cpu_address_absolute_x_notload(exec) { \
  uint32_t op_address = (instr_fetchw(PCreg) + Xreg) & 0xffff; \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_absolute_y_notload(exec) { \
  uint32_t op_address = (instr_fetchw(PCreg) + Yreg) & 0xFFFF; \
  PCreg += 2; \
  exec(false); \
}

#define cpu_address_indirect_x_notload(exec) { \
  uint32_t arg_addr = (instr_fetch(PCreg) + Xreg) & 0xFF; \
  assert(0); \
  uint32_t op_address = cpu_ram_readw(arg_addr); \
  PCreg++; \
  exec(false); \
}

#define cpu_address_indirect_y_notload(exec) { \
  uint32_t arg_addr = instr_fetch(PCreg); \
  uint32_t temp = cpu_ram_readw(arg_addr); \
  uint32_t op_address = (temp + (Yreg & 0xff)) & 0xFFFF; \
  PCreg++; \
  exec(false); \
}


// CPU Memory

byte CPU_RAM[0x8000];

inline uint32_t cpu_ram_read(uint32_t address) {
  return CPU_RAM[address];
}

inline void cpu_ram_write(uint32_t address, uint32_t byte_data) {
  CPU_RAM[address] = byte_data;
}

static inline uint32_t cpu_ram_readw(uint32_t address) {
  return cpu_ram_read(address) | (cpu_ram_read(address + 1) << 8);
}

static inline void cpu_ram_writew(uint32_t address, uint32_t word_data) {
  cpu_ram_write(address, word_data & 0xff);
  cpu_ram_write(address + 1, word_data >> 8);
}


// Interrupt Addresses

static inline uint32_t cpu_nmi_interrupt_address()   { return memory_readw(0xFFFA); }
static inline uint32_t cpu_reset_interrupt_address() { return memory_readw(0xFFFC); }
static inline uint32_t cpu_irq_interrupt_address()   { return memory_readw(0xFFFE); }



// Stack Routines

static inline void cpu_stack_pushb(uint32_t byte_data) { cpu_ram_write(0x100 + cpu.SP--, byte_data);       }
static inline void cpu_stack_pushw(uint32_t word_data) { cpu_ram_writew(0xFF + cpu.SP, word_data); cpu.SP -= 2; }
static inline uint32_t cpu_stack_popb()           { return cpu_ram_read(0x100 + ++cpu.SP);       }
static inline uint32_t cpu_stack_popw()           { cpu.SP += 2; return cpu_ram_readw(0xFF + cpu.SP); }



// flags

// lazy flag computation

enum { FLAG_STATE_UPTODATE, FLAG_STATE_LAZY = 1, FLAG_STATE_OP };

static int g_lz_zn_result = FLAG_STATE_UPTODATE; // for zero and negative flags
int lz_ov_result, lz_ov_src1, lz_ov_src1, lz_ov_state; // for overflow flag
int lz_c_result, lz_c_src1, lz_c_src1, lz_c_state; // for carry flag

#define lz_is_n_lazy(lz_zn_result) (lz_zn_result & (1 << 25))

#define cpu_flag_set(flag) cpu.P[flag]
#define cpu_flag_setbit(flag) !!cpu.P[flag]
#define cpu_modify_flag(flag, value) cpu.P[flag] = (value)
#define cpu_set_flag(flag) cpu.P[flag] = 1
#define cpu_unset_flag(flag) cpu.P[flag] = 0

#define cpu_update_zn_flags(value) { \
  lz_zn_result = (value) | (1 << 25); \
  /* cpu_modify_flag(zero_bp, (value & 0xff) == 0); */ \
  /* cpu_modify_flag(negative_bp, (value >> 7) & 1); */ \
}

#define lz_set_n_uptodate(lz_zn_result) { lz_zn_result &= ~(1 << 25); }

#define lz_query_z(lz_zn_result) ((lz_zn_result & 0xff) == 0)

#define lz_compute_z(lz_zn_result) { \
  cpu_modify_flag(zero_bp, lz_query_z(lz_zn_result)); \
}

#define lz_compute_n(lz_zn_result) { \
  if (lz_is_n_lazy(lz_zn_result)) { \
    cpu_modify_flag(negative_bp, lz_zn_result & 0x80); \
    lz_set_n_uptodate(lz_zn_result); \
  } \
}

// CPU Instructions

static inline void ____FE____() { /* Instruction for future Extension */ }

// NOP

#define cpu_op_nop(in_zero_page) { }

// Addition

#define cpu_op_adc(in_zero_page) { \
    int result = (Areg & 0xff) + op_value + cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, result & 0x100); \
    cpu_modify_flag(overflow_bp, ~(Areg ^ op_value) & (Areg ^ result) & 0x80); \
    cpu_update_zn_flags(result); \
    Areg = result; \
}

// Subtraction

#define cpu_op_sbc(in_zero_page) { \
    uint32_t result = (Areg & 0xff) - op_value - !cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, !(result & 0x100)); \
    cpu_modify_flag(overflow_bp, (Areg ^ op_value) & (Areg ^ result) & 0x80); \
    cpu_update_zn_flags(result & 0xff); \
    Areg = result; \
}

// Bit Manipulation Operations

#define cpu_op_and(in_zero_page) { cpu_update_zn_flags(Areg &= op_value); }
#define cpu_op_bit(in_zero_page) { \
  cpu_modify_flag(overflow_bp, op_value & 0x40); \
  /*cpu_modify_flag(zero_bp, (Areg & op_value & 0xff) == 0); */\
  cpu_update_zn_flags(Areg & op_value); \
  cpu_modify_flag(negative_bp, op_value & 0x80); \
  lz_set_n_uptodate(lz_zn_result); \
}
#define cpu_op_eor(in_zero_page) { cpu_update_zn_flags(Areg ^= op_value); }
#define cpu_op_ora(in_zero_page) { cpu_update_zn_flags(Areg |= op_value); }
#define cpu_op_asla(in_zero_page) { \
    cpu_modify_flag(carry_bp, Areg & 0x80); \
    Areg <<= 1; \
    cpu_update_zn_flags(Areg); \
}
#define cpu_op_asl(in_zero_page) { \
    cpu_modify_flag(carry_bp, op_value & 0x80); \
    op_value <<= 1; \
    /*op_value &= 0xFF; */\
    cpu_update_zn_flags(op_value); \
    if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
    else { memory_writeb(op_address, op_value); } \
}
#define cpu_op_lsra(in_zero_page) { \
    uint32_t value = (Areg & 0xff) >> 1; \
    cpu_modify_flag(carry_bp, Areg & 0x01); \
    cpu_update_zn_flags(value); \
    Areg = value; \
}
#define cpu_op_lsr(in_zero_page) { \
    cpu_modify_flag(carry_bp, op_value & 0x01); \
    op_value >>= 1; \
    /*op_value &= 0xFF; */\
    cpu_update_zn_flags(op_value); \
    if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
    else { memory_writeb(op_address, op_value); } \
}

#define cpu_op_rola(in_zero_page) { \
    uint32_t value = (Areg & 0xff) << 1; \
    value |= cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, value & 0x100); \
    cpu_update_zn_flags(value); \
    Areg = value; \
}
#define cpu_op_rol(in_zero_page) { \
    op_value <<= 1; \
    op_value |= cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, op_value & 0x100); \
    /* op_value &= 0xFF; */\
    cpu_update_zn_flags(op_value); \
    if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
    else { memory_writeb(op_address, op_value); } \
}
#define cpu_op_rora(in_zero_page) { \
    unsigned char carry = cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, Areg & 0x01); \
    Areg = ((Areg & 0xff) >> 1) | (carry << 7); \
    cpu_update_zn_flags(Areg); \
}
#define cpu_op_ror(in_zero_page) { \
    unsigned char carry = cpu_flag_setbit(carry_bp); \
    cpu_modify_flag(carry_bp, op_value & 0x01); \
    op_value = (op_value >> 1) | (carry << 7); \
    cpu_update_zn_flags(op_value); \
    if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
    else { memory_writeb(op_address, op_value); } \
}

// Loading

#define cpu_op_lda(in_zero_page) { cpu_update_zn_flags(Areg = op_value); }
#define cpu_op_ldx(in_zero_page) { cpu_update_zn_flags(Xreg = op_value); }
#define cpu_op_ldy(in_zero_page) { cpu_update_zn_flags(Yreg = op_value); }

// Storing

#define cpu_op_sta(in_zero_page) { \
  if (in_zero_page) { cpu_ram_write(op_address, Areg); } \
  else { memory_writeb(op_address, Areg); } \
}

#define cpu_op_stx(in_zero_page) { \
  if (in_zero_page) { cpu_ram_write(op_address, Xreg); } \
  else { memory_writeb(op_address, Xreg); } \
}

#define cpu_op_sty(in_zero_page) { \
  if (in_zero_page) { cpu_ram_write(op_address, Yreg); } \
  else { memory_writeb(op_address, Yreg); } \
}

// Transfering

#define cpu_op_tax(in_zero_page) { cpu_update_zn_flags(Xreg = Areg);  }
#define cpu_op_txa(in_zero_page) { cpu_update_zn_flags(Areg = Xreg);  }
#define cpu_op_tay(in_zero_page) { cpu_update_zn_flags(Yreg = Areg);  }
#define cpu_op_tya(in_zero_page) { cpu_update_zn_flags(Areg = Yreg);  }
#define cpu_op_tsx(in_zero_page) { cpu_update_zn_flags(Xreg = cpu.SP); }
#define cpu_op_txs(in_zero_page) { cpu.SP = Xreg & 0xff; }

#define cpu_branch(flag) do { \
  if (flag) { \
    int8_t offset = instr_fetch(PCreg); \
    PCreg += offset + 1; \
  } \
  else PCreg ++; \
} while(0)

// Branching Positive

#define cpu_op_bcs(in_zero_page) { cpu_branch(cpu_flag_set(carry_bp));     }
#define cpu_op_beq(in_zero_page) { cpu_branch(lz_query_z(lz_zn_result));      }
#define cpu_op_bmi(in_zero_page) { lz_compute_n(lz_zn_result); cpu_branch(cpu_flag_set(negative_bp));  }
#define cpu_op_bvs(in_zero_page) { cpu_branch(cpu_flag_set(overflow_bp));  }

// Branching Negative

#define cpu_op_bne(in_zero_page) { cpu_branch(!lz_query_z(lz_zn_result));     }
#define cpu_op_bcc(in_zero_page) { cpu_branch(!cpu_flag_set(carry_bp));    }
#define cpu_op_bpl(in_zero_page) { lz_compute_n(lz_zn_result); cpu_branch(!cpu_flag_set(negative_bp)); }
#define cpu_op_bvc(in_zero_page) { cpu_branch(!cpu_flag_set(overflow_bp)); }

// Jumping

#define cpu_op_jmp(in_zero_page) { PCreg = op_address; }

// Subroutines

#define cpu_op_jsr(in_zero_page) { cpu_stack_pushw(PCreg + 1); PCreg = op_address; }
#define cpu_op_rts(in_zero_page) { PCreg = cpu_stack_popw(in_zero_page) + 1; }

// Interruptions

#define cpu_op_brk(in_zero_page) { \
  cpu_stack_pushw(PCreg - 1); \
  lz_compute_n(lz_zn_result); \
  lz_compute_z(lz_zn_result); \
  cpu_stack_pushb(byte_pack(cpu.P)); \
  cpu.P[unused_bp] = 1; \
  cpu.P[break_bp] = 1; \
  PCreg = cpu_nmi_interrupt_address(); \
}
#define cpu_op_rti(in_zero_page) { \
  byte_unpack(cpu.P, cpu_stack_popb()); \
  cpu_update_zn_flags(!cpu.P[zero_bp]); \
  lz_set_n_uptodate(lz_zn_result); \
  cpu.P[unused_bp] = 1; \
	PCreg = cpu_stack_popw(); \
}

// Flags

#define cpu_op_clc(in_zero_page) { cpu_unset_flag(carry_bp);     }
#define cpu_op_cld(in_zero_page) { cpu_unset_flag(decimal_bp);   }
#define cpu_op_cli(in_zero_page) { cpu_unset_flag(interrupt_bp); }
#define cpu_op_clv(in_zero_page) { cpu_unset_flag(overflow_bp);  }
#define cpu_op_sec(in_zero_page) { cpu_set_flag(carry_bp);       }
#define cpu_op_sed(in_zero_page) { cpu_set_flag(decimal_bp);     }
#define cpu_op_sei(in_zero_page) { cpu_set_flag(interrupt_bp);   }

// Comparison

#define cpu_compare(reg) int result = (reg & 0xff) - op_value; \
                         cpu_modify_flag(carry_bp, result >= 0); \
                         cpu_update_zn_flags(result & 0xff);

#define cpu_op_cmp(in_zero_page) { cpu_compare(Areg); }
#define cpu_op_cpx(in_zero_page) { cpu_compare(Xreg); }
#define cpu_op_cpy(in_zero_page) { cpu_compare(Yreg); }

// Increment

#define cpu_op_inc(in_zero_page) { \
  uint32_t result = op_value + 1; \
  if (in_zero_page) { cpu_ram_write(op_address, result); } \
  else { memory_writeb(op_address, result); } \
  cpu_update_zn_flags(result); \
}
#define cpu_op_inx(in_zero_page) { Xreg = (Xreg + 1) & 0xff; cpu_update_zn_flags(Xreg); }
#define cpu_op_iny(in_zero_page) { Yreg = (Yreg + 1) & 0xff; cpu_update_zn_flags(Yreg); }

// Decrement

#define cpu_op_dec(in_zero_page) { \
  uint32_t result = (op_value - 1) & 0xff; \
  if (in_zero_page) { cpu_ram_write(op_address, result); } \
  else { memory_writeb(op_address, result); } \
  cpu_update_zn_flags(result); \
}
#define cpu_op_dex(in_zero_page) { Xreg = (Xreg - 1) & 0xff; cpu_update_zn_flags(Xreg); }
#define cpu_op_dey(in_zero_page) { Yreg = (Yreg - 1) & 0xff; cpu_update_zn_flags(Yreg); }

// Stack

#define cpu_op_php(in_zero_page) { lz_compute_z(lz_zn_result); lz_compute_n(lz_zn_result); cpu_stack_pushb(byte_pack(cpu.P) | 0x30); }
#define cpu_op_pha(in_zero_page) { cpu_stack_pushb(Areg & 0xff); }
#define cpu_op_pla(in_zero_page) { Areg = cpu_stack_popb(); cpu_update_zn_flags(Areg); }
#define cpu_op_plp(in_zero_page) { \
/*  cpu.P = (cpu_stack_popb() & 0xEF) | 0x20; */ \
  byte_unpack(cpu.P, cpu_stack_popb()); \
  cpu.P[break_bp] = 0; \
  cpu.P[unused_bp] = 1; \
  cpu_update_zn_flags(!cpu.P[zero_bp]); \
  lz_set_n_uptodate(lz_zn_result); \
}



// Extended Instruction Set

#define cpu_op_aso(in_zero_page) { cpu_op_asl(in_zero_page); cpu_op_ora(in_zero_page); }
#define cpu_op_axa(in_zero_page) { \
  uint32_t result = Areg & (Xreg & 0xff) & (op_address >> 8); \
  if (in_zero_page) { cpu_ram_write(op_address, result); } \
  else { memory_writeb(op_address, result); } \
}
#define cpu_op_axs(in_zero_page) { \
  uint32_t result = Areg & (Xreg & 0xff); \
  if (in_zero_page) { cpu_ram_write(op_address, result); } \
  else { memory_writeb(op_address, result); } \
}
#define cpu_op_dcm(in_zero_page) { \
  op_value--;\
  op_value &= 0xFF; \
  if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
  else { memory_writeb(op_address, op_value); } \
  cpu_op_cmp(in_zero_page); \
}
#define cpu_op_ins(in_zero_page) { \
  op_value = (op_value + 1) & 0xFF; \
  if (in_zero_page) { cpu_ram_write(op_address, op_value); } \
  else { memory_writeb(op_address, op_value); } \
  cpu_op_sbc(in_zero_page); \
}
#define cpu_op_lax(in_zero_page) { cpu_update_zn_flags(Areg = Xreg = op_value & 0xff); }
#define cpu_op_lse(in_zero_page) { cpu_op_lsr(in_zero_page); cpu_op_eor(in_zero_page); }
#define cpu_op_rla(in_zero_page) { cpu_op_rol(in_zero_page); cpu_op_and(in_zero_page); }
#define cpu_op_rra(in_zero_page) { cpu_op_ror(in_zero_page); cpu_op_adc(in_zero_page); }




#if 0
// Base 6502 instruction set

#define CPU_OP_BIS(o, c, f, n, a) cpu_op_cycles[0x##o] = c; \
                                  cpu_op_handler[0x##o] = cpu_op_##f; \
                                  /*cpu_op_name[0x##o] = n; */\
                                  cpu_op_address_mode[0x##o] = cpu_address_##a; \
                                  /*cpu_op_in_base_instruction_set[0x##o] = true; */

// Not implemented instructions

#define CPU_OP_NII(o, a) cpu_op_cycles[0x##o] = 1; \
                         cpu_op_handler[0x##o] = ____FE____; \
                         /*cpu_op_name[0x##o] = "NOP"; */\
                         cpu_op_address_mode[0x##o] = cpu_address_##a; \
                         /*cpu_op_in_base_instruction_set[0x##o] = false; */

// Extended instruction set found in other CPUs and implemented for compatibility

#define CPU_OP_EIS(o, c, f, n, a) cpu_op_cycles[0x##o] = c; \
                                  cpu_op_handler[0x##o] = cpu_op_##f; \
                                  /*cpu_op_name[0x##o] = n; */ \
                                  cpu_op_address_mode[0x##o] = cpu_address_##a; \
                                  /*cpu_op_in_base_instruction_set[0x##o] = false;*/
#endif


#define CASE_CPU_OP_BIS(o, c, f, n, a) \
  case 0x##o: cpu_address_##a(cpu_op_##f); break;

#define CASE_CPU_OP_NII(o, a) \
  case 0x##o: cpu_address_##a(____FE____); break;

#define CASE_CPU_OP_EIS(o, c, f, n, a) CASE_CPU_OP_BIS(o, c, f, n, a)


// CPU Lifecycle

void cpu_init()
{
    //cpu.P = 0x24;
    cpu.P[interrupt_bp] = 1;
    cpu.P[unused_bp] = 1;
    cpu.SP = 0x00;
    cpu.A = cpu.X = cpu.Y = 0;
}

void cpu_reset()
{
    cpu.PC = cpu_reset_interrupt_address();
    cpu.SP -= 3;
    cpu.P[interrupt_bp] = 1;
}

#ifdef STATISTIC
static int cpu_op_cnts[256];
static void display_statistic() {
  int i;
  int total = 0;
  for (i = 0; i < 256; i += 8) {
    total += cpu_op_cnts[i + 0] + cpu_op_cnts[i + 1] + cpu_op_cnts[i + 2] + cpu_op_cnts[i + 3] +
      cpu_op_cnts[i + 4] + cpu_op_cnts[i + 5] + cpu_op_cnts[i + 6] + cpu_op_cnts[i + 7];
  }

  for (i = 0; i < 256; i ++) {
    cpu_op_cnts[i] = (cpu_op_cnts[i] * 1000) / total;
  }

  for (i = 0; i < 256; i += 8) {
    printf("0x%02x: %8d %8d %8d %8d    %8d %8d %8d %8d\n", i,
        cpu_op_cnts[i + 0], cpu_op_cnts[i + 1], cpu_op_cnts[i + 2], cpu_op_cnts[i + 3],
        cpu_op_cnts[i + 4], cpu_op_cnts[i + 5], cpu_op_cnts[i + 6], cpu_op_cnts[i + 7]);
  }

  for (i = 0; i < 256; i ++) {
    cpu_op_cnts[i] = 0;
  }

  printf("========== Total = %d ===========\n", total);
}
#endif

void cpu_interrupt()
{
  // if (ppu_in_vblank()) {
  if (ppu_generates_nmi()) {
    lz_compute_n(g_lz_zn_result);
    lz_compute_z(g_lz_zn_result);
    cpu.P[interrupt_bp] = 1;
    cpu_unset_flag(unused_bp);
    cpu_stack_pushw(cpu.PC);
    cpu_stack_pushb(byte_pack(cpu.P));
    cpu.PC = cpu_nmi_interrupt_address();
  }
  // }
}

inline unsigned long cpu_clock()
{
    return cpu_cycles;
}

void cpu_run(long cycles)
{
  int lz_zn_result = g_lz_zn_result; // for zero and negative flags
  uint32_t PCreg = cpu.PC, Areg = cpu.A, Xreg = cpu.X, Yreg = cpu.Y;
  cycles /= 3;
  long c = cycles;
  while (cycles > 0) {
    uint32_t op_code = instr_fetch(PCreg++);
    int op_cycles = cycle_table[op_code];
      switch (op_code) {
        CASE_CPU_OP_BIS(00, 7, brk, "BRK", implied)
        CASE_CPU_OP_BIS(01, 6, ora, "ORA", indirect_x)
        CASE_CPU_OP_BIS(05, 3, ora, "ORA", zero_page)
        CASE_CPU_OP_BIS(06, 5, asl, "ASL", zero_page)
        CASE_CPU_OP_BIS(08, 3, php, "PHP", implied)
        CASE_CPU_OP_BIS(09, 2, ora, "ORA", immediate)
        CASE_CPU_OP_BIS(0A, 2, asla,"ASL", implied)
        CASE_CPU_OP_BIS(0D, 4, ora, "ORA", absolute)
        CASE_CPU_OP_BIS(0E, 6, asl, "ASL", absolute)
        CASE_CPU_OP_BIS(10, 2, bpl, "BPL", relative)
        CASE_CPU_OP_BIS(11, 5, ora, "ORA", indirect_y)
        CASE_CPU_OP_BIS(15, 4, ora, "ORA", zero_page_x)
        CASE_CPU_OP_BIS(16, 6, asl, "ASL", zero_page_x)
        CASE_CPU_OP_BIS(18, 2, clc, "CLC", implied)
        CASE_CPU_OP_BIS(19, 4, ora, "ORA", absolute_y)
        CASE_CPU_OP_BIS(1D, 4, ora, "ORA", absolute_x)
        CASE_CPU_OP_BIS(1E, 7, asl, "ASL", absolute_x)
        CASE_CPU_OP_BIS(20, 6, jsr, "JSR", absolute_jmp)
        CASE_CPU_OP_BIS(21, 6, and, "AND", indirect_x)
        CASE_CPU_OP_BIS(24, 3, bit, "BIT", zero_page)
        CASE_CPU_OP_BIS(25, 3, and, "AND", zero_page)
        CASE_CPU_OP_BIS(26, 5, rol, "ROL", zero_page)
        CASE_CPU_OP_BIS(28, 4, plp, "PLP", implied)
        CASE_CPU_OP_BIS(29, 2, and, "AND", immediate)
        CASE_CPU_OP_BIS(2A, 2, rola,"ROL", implied)
        CASE_CPU_OP_BIS(2C, 4, bit, "BIT", absolute)
        CASE_CPU_OP_BIS(2D, 2, and, "AND", absolute)
        CASE_CPU_OP_BIS(2E, 6, rol, "ROL", absolute)
        CASE_CPU_OP_BIS(30, 2, bmi, "BMI", relative)
        CASE_CPU_OP_BIS(31, 5, and, "AND", indirect_y)
        CASE_CPU_OP_BIS(35, 4, and, "AND", zero_page_x)
        CASE_CPU_OP_BIS(36, 6, rol, "ROL", zero_page_x)
        CASE_CPU_OP_BIS(38, 2, sec, "SEC", implied)
        CASE_CPU_OP_BIS(39, 4, and, "AND", absolute_y)
        CASE_CPU_OP_BIS(3D, 4, and, "AND", absolute_x)
        CASE_CPU_OP_BIS(3E, 7, rol, "ROL", absolute_x)
        CASE_CPU_OP_BIS(40, 6, rti, "RTI", implied)
        CASE_CPU_OP_BIS(41, 6, eor, "EOR", indirect_x)
        CASE_CPU_OP_BIS(45, 3, eor, "EOR", zero_page)
        CASE_CPU_OP_BIS(46, 5, lsr, "LSR", zero_page)
        CASE_CPU_OP_BIS(48, 3, pha, "PHA", implied)
        CASE_CPU_OP_BIS(49, 2, eor, "EOR", immediate)
        CASE_CPU_OP_BIS(4A, 2, lsra,"LSR", implied)
        CASE_CPU_OP_BIS(4C, 3, jmp, "JMP", absolute_jmp)
        CASE_CPU_OP_BIS(4D, 4, eor, "EOR", absolute)
        CASE_CPU_OP_BIS(4E, 6, lsr, "LSR", absolute)
        CASE_CPU_OP_BIS(50, 2, bvc, "BVC", relative)
        CASE_CPU_OP_BIS(51, 5, eor, "EOR", indirect_y)
        CASE_CPU_OP_BIS(55, 4, eor, "EOR", zero_page_x)
        CASE_CPU_OP_BIS(56, 6, lsr, "LSR", zero_page_x)
        CASE_CPU_OP_BIS(58, 2, cli, "CLI", implied)
        CASE_CPU_OP_BIS(59, 4, eor, "EOR", absolute_y)
        CASE_CPU_OP_BIS(5D, 4, eor, "EOR", absolute_x)
        CASE_CPU_OP_BIS(5E, 7, lsr, "LSR", absolute_x)
        CASE_CPU_OP_BIS(60, 6, rts, "RTS", implied)
        CASE_CPU_OP_BIS(61, 6, adc, "ADC", indirect_x)
        CASE_CPU_OP_BIS(65, 3, adc, "ADC", zero_page)
        CASE_CPU_OP_BIS(66, 5, ror, "ROR", zero_page)
        CASE_CPU_OP_BIS(68, 4, pla, "PLA", implied)
        CASE_CPU_OP_BIS(69, 2, adc, "ADC", immediate)
        CASE_CPU_OP_BIS(6A, 2, rora,"ROR", implied)
        CASE_CPU_OP_BIS(6C, 5, jmp, "JMP", indirect)
        CASE_CPU_OP_BIS(6D, 4, adc, "ADC", absolute)
        CASE_CPU_OP_BIS(6E, 6, ror, "ROR", absolute)
        CASE_CPU_OP_BIS(70, 2, bvs, "BVS", relative)
        CASE_CPU_OP_BIS(71, 5, adc, "ADC", indirect_y)
        CASE_CPU_OP_BIS(75, 4, adc, "ADC", zero_page_x)
        CASE_CPU_OP_BIS(76, 6, ror, "ROR", zero_page_x)
        CASE_CPU_OP_BIS(78, 2, sei, "SEI", implied)
        CASE_CPU_OP_BIS(79, 4, adc, "ADC", absolute_y)
        CASE_CPU_OP_BIS(7D, 4, adc, "ADC", absolute_x)
        CASE_CPU_OP_BIS(7E, 7, ror, "ROR", absolute_x)
        CASE_CPU_OP_BIS(81, 6, sta, "STA", indirect_x_notload)
        CASE_CPU_OP_BIS(84, 3, sty, "STY", zero_page_notload)
        CASE_CPU_OP_BIS(85, 3, sta, "STA", zero_page_notload)
        CASE_CPU_OP_BIS(86, 3, stx, "STX", zero_page_notload)
        CASE_CPU_OP_BIS(88, 2, dey, "DEY", implied)
        CASE_CPU_OP_BIS(8A, 2, txa, "TXA", implied)
        CASE_CPU_OP_BIS(8C, 4, sty, "STY", absolute_notload)
        CASE_CPU_OP_BIS(8D, 4, sta, "STA", absolute_notload)
        CASE_CPU_OP_BIS(8E, 4, stx, "STX", absolute_notload)
        CASE_CPU_OP_BIS(90, 2, bcc, "BCC", relative)
        CASE_CPU_OP_BIS(91, 6, sta, "STA", indirect_y_notload)
        CASE_CPU_OP_BIS(94, 4, sty, "STY", zero_page_x_notload)
        CASE_CPU_OP_BIS(95, 4, sta, "STA", zero_page_x_notload)
        CASE_CPU_OP_BIS(96, 4, stx, "STX", zero_page_y_notload)
        CASE_CPU_OP_BIS(98, 2, tya, "TYA", implied)
        CASE_CPU_OP_BIS(99, 5, sta, "STA", absolute_y_notload)
        CASE_CPU_OP_BIS(9A, 2, txs, "TXS", implied)
        CASE_CPU_OP_BIS(9D, 5, sta, "STA", absolute_x_notload)
        CASE_CPU_OP_BIS(A0, 2, ldy, "LDY", immediate)
        CASE_CPU_OP_BIS(A1, 6, lda, "LDA", indirect_x)
        CASE_CPU_OP_BIS(A2, 2, ldx, "LDX", immediate)
        CASE_CPU_OP_BIS(A4, 3, ldy, "LDY", zero_page)
        CASE_CPU_OP_BIS(A5, 3, lda, "LDA", zero_page)
        CASE_CPU_OP_BIS(A6, 3, ldx, "LDX", zero_page)
        CASE_CPU_OP_BIS(A8, 2, tay, "TAY", implied)
        CASE_CPU_OP_BIS(A9, 2, lda, "LDA", immediate)
        CASE_CPU_OP_BIS(AA, 2, tax, "TAX", implied)
        CASE_CPU_OP_BIS(AC, 4, ldy, "LDY", absolute)
        CASE_CPU_OP_BIS(AD, 4, lda, "LDA", absolute)
        CASE_CPU_OP_BIS(AE, 4, ldx, "LDX", absolute)
        CASE_CPU_OP_BIS(B0, 2, bcs, "BCS", relative)
        CASE_CPU_OP_BIS(B1, 5, lda, "LDA", indirect_y)
        CASE_CPU_OP_BIS(B4, 4, ldy, "LDY", zero_page_x)
        CASE_CPU_OP_BIS(B5, 4, lda, "LDA", zero_page_x)
        CASE_CPU_OP_BIS(B6, 4, ldx, "LDX", zero_page_y)
        CASE_CPU_OP_BIS(B8, 2, clv, "CLV", implied)
        CASE_CPU_OP_BIS(B9, 4, lda, "LDA", absolute_y)
        CASE_CPU_OP_BIS(BA, 2, tsx, "TSX", implied)
        CASE_CPU_OP_BIS(BC, 4, ldy, "LDY", absolute_x)
        CASE_CPU_OP_BIS(BD, 4, lda, "LDA", absolute_x)
        CASE_CPU_OP_BIS(BE, 4, ldx, "LDX", absolute_y)
        CASE_CPU_OP_BIS(C0, 2, cpy, "CPY", immediate)
        CASE_CPU_OP_BIS(C1, 6, cmp, "CMP", indirect_x)
        CASE_CPU_OP_BIS(C4, 3, cpy, "CPY", zero_page)
        CASE_CPU_OP_BIS(C5, 3, cmp, "CMP", zero_page)
        CASE_CPU_OP_BIS(C6, 5, dec, "DEC", zero_page)
        CASE_CPU_OP_BIS(C8, 2, iny, "INY", implied)
        CASE_CPU_OP_BIS(C9, 2, cmp, "CMP", immediate)
        CASE_CPU_OP_BIS(CA, 2, dex, "DEX", implied)
        CASE_CPU_OP_BIS(CC, 4, cpy, "CPY", absolute)
        CASE_CPU_OP_BIS(CD, 4, cmp, "CMP", absolute)
        CASE_CPU_OP_BIS(CE, 6, dec, "DEC", absolute)
        CASE_CPU_OP_BIS(D0, 2, bne, "BNE", relative)
        CASE_CPU_OP_BIS(D1, 5, cmp, "CMP", indirect_y)
        CASE_CPU_OP_BIS(D5, 4, cmp, "CMP", zero_page_x)
        CASE_CPU_OP_BIS(D6, 6, dec, "DEC", zero_page_x)
        CASE_CPU_OP_BIS(D8, 2, cld, "CLD", implied)
        CASE_CPU_OP_BIS(D9, 4, cmp, "CMP", absolute_y)
        CASE_CPU_OP_BIS(DD, 4, cmp, "CMP", absolute_x)
        CASE_CPU_OP_BIS(DE, 7, dec, "DEC", absolute_x)
        CASE_CPU_OP_BIS(E0, 2, cpx, "CPX", immediate)
        CASE_CPU_OP_BIS(E1, 6, sbc, "SBC", indirect_x)
        CASE_CPU_OP_BIS(E4, 3, cpx, "CPX", zero_page)
        CASE_CPU_OP_BIS(E5, 3, sbc, "SBC", zero_page)
        CASE_CPU_OP_BIS(E6, 5, inc, "INC", zero_page)
        CASE_CPU_OP_BIS(E8, 2, inx, "INX", implied)
        CASE_CPU_OP_BIS(E9, 2, sbc, "SBC", immediate)
        CASE_CPU_OP_BIS(EA, 2, nop, "NOP", implied)
        CASE_CPU_OP_BIS(EC, 4, cpx, "CPX", absolute)
        CASE_CPU_OP_BIS(ED, 4, sbc, "SBC", absolute)
        CASE_CPU_OP_BIS(EE, 6, inc, "INC", absolute)
        CASE_CPU_OP_BIS(F0, 2, beq, "BEQ", relative)
        CASE_CPU_OP_BIS(F1, 5, sbc, "SBC", indirect_y)
        CASE_CPU_OP_BIS(F5, 4, sbc, "SBC", zero_page_x)
        CASE_CPU_OP_BIS(F6, 6, inc, "INC", zero_page_x)
        CASE_CPU_OP_BIS(F8, 2, sed, "SED", implied)
        CASE_CPU_OP_BIS(F9, 4, sbc, "SBC", absolute_y)
        CASE_CPU_OP_BIS(FD, 4, sbc, "SBC", absolute_x)
        CASE_CPU_OP_BIS(FE, 7, inc, "INC", absolute_x)

        CASE_CPU_OP_EIS(03, 8, aso, "SLO", indirect_x)
        CASE_CPU_OP_EIS(07, 5, aso, "SLO", zero_page)
        CASE_CPU_OP_EIS(0F, 6, aso, "SLO", absolute)
        CASE_CPU_OP_EIS(13, 8, aso, "SLO", indirect_y)
        CASE_CPU_OP_EIS(17, 6, aso, "SLO", zero_page_x)
        CASE_CPU_OP_EIS(1B, 7, aso, "SLO", absolute_y)
        CASE_CPU_OP_EIS(1F, 7, aso, "SLO", absolute_x)
        CASE_CPU_OP_EIS(23, 8, rla, "RLA", indirect_x)
        CASE_CPU_OP_EIS(27, 5, rla, "RLA", zero_page)
        CASE_CPU_OP_EIS(2F, 6, rla, "RLA", absolute)
        CASE_CPU_OP_EIS(33, 8, rla, "RLA", indirect_y)
        CASE_CPU_OP_EIS(37, 6, rla, "RLA", zero_page_x)
        CASE_CPU_OP_EIS(3B, 7, rla, "RLA", absolute_y)
        CASE_CPU_OP_EIS(3F, 7, rla, "RLA", absolute_x)
        CASE_CPU_OP_EIS(43, 8, lse, "SRE", indirect_x)
        CASE_CPU_OP_EIS(47, 5, lse, "SRE", zero_page)
        CASE_CPU_OP_EIS(4F, 6, lse, "SRE", absolute)
        CASE_CPU_OP_EIS(53, 8, lse, "SRE", indirect_y)
        CASE_CPU_OP_EIS(57, 6, lse, "SRE", zero_page_x)
        CASE_CPU_OP_EIS(5B, 7, lse, "SRE", absolute_y)
        CASE_CPU_OP_EIS(5F, 7, lse, "SRE", absolute_x)
        CASE_CPU_OP_EIS(63, 8, rra, "RRA", indirect_x)
        CASE_CPU_OP_EIS(67, 5, rra, "RRA", zero_page)
        CASE_CPU_OP_EIS(6F, 6, rra, "RRA", absolute)
        CASE_CPU_OP_EIS(73, 8, rra, "RRA", indirect_y)
        CASE_CPU_OP_EIS(77, 6, rra, "RRA", zero_page_x)
        CASE_CPU_OP_EIS(7B, 7, rra, "RRA", absolute_y)
        CASE_CPU_OP_EIS(7F, 7, rra, "RRA", absolute_x)
        CASE_CPU_OP_EIS(83, 6, axs, "SAX", indirect_x_notload)
        CASE_CPU_OP_EIS(87, 3, axs, "SAX", zero_page_notload)
        CASE_CPU_OP_EIS(8F, 4, axs, "SAX", absolute_notload)
        CASE_CPU_OP_EIS(93, 6, axa, "SAX", indirect_y_notload)
        CASE_CPU_OP_EIS(97, 4, axs, "SAX", zero_page_y_notload)
        CASE_CPU_OP_EIS(9F, 5, axa, "SAX", absolute_y_notload)
        CASE_CPU_OP_EIS(A3, 6, lax, "LAX", indirect_x)
        CASE_CPU_OP_EIS(A7, 3, lax, "LAX", zero_page)
        CASE_CPU_OP_EIS(AF, 4, lax, "LAX", absolute)
        CASE_CPU_OP_EIS(B3, 5, lax, "LAX", indirect_y)
        CASE_CPU_OP_EIS(B7, 4, lax, "LAX", zero_page_y)
        CASE_CPU_OP_EIS(BF, 4, lax, "LAX", absolute_y)
        CASE_CPU_OP_EIS(C3, 8, dcm, "DCP", indirect_x)
        CASE_CPU_OP_EIS(C7, 5, dcm, "DCP", zero_page)
        CASE_CPU_OP_EIS(CF, 6, dcm, "DCP", absolute)
        CASE_CPU_OP_EIS(D3, 8, dcm, "DCP", indirect_y)
        CASE_CPU_OP_EIS(D7, 6, dcm, "DCP", zero_page_x)
        CASE_CPU_OP_EIS(DB, 7, dcm, "DCP", absolute_y)
        CASE_CPU_OP_EIS(DF, 7, dcm, "DCP", absolute_x)
        CASE_CPU_OP_EIS(E3, 8, ins, "ISB", indirect_x)
        CASE_CPU_OP_EIS(E7, 5, ins, "ISB", zero_page)
        CASE_CPU_OP_EIS(EB, 2, sbc, "SBC", immediate)
        CASE_CPU_OP_EIS(EF, 6, ins, "ISB", absolute)
        CASE_CPU_OP_EIS(F3, 8, ins, "ISB", indirect_y)
        CASE_CPU_OP_EIS(F7, 6, ins, "ISB", zero_page_x)
        CASE_CPU_OP_EIS(FB, 7, ins, "ISB", absolute_y)
        CASE_CPU_OP_EIS(FF, 7, ins, "ISB", absolute_x)

        default: printf("not implemented opcode = 0x%02x\n", op_code); assert(0);
        //CASE_CPU_OP_NII(04, zero_page)
        //CASE_CPU_OP_NII(0C, absolute)
        //CASE_CPU_OP_NII(14, zero_page_x)
        //CASE_CPU_OP_NII(1A, implied)
        //CASE_CPU_OP_NII(1C, absolute_x)
        //CASE_CPU_OP_NII(34, zero_page_x)
        //CASE_CPU_OP_NII(3A, implied)
        //CASE_CPU_OP_NII(3C, absolute_x)
        //CASE_CPU_OP_NII(44, zero_page)
        //CASE_CPU_OP_NII(54, zero_page_x)
        //CASE_CPU_OP_NII(5A, implied)
        //CASE_CPU_OP_NII(5C, absolute_x)
        //CASE_CPU_OP_NII(64, zero_page)
        //CASE_CPU_OP_NII(74, zero_page_x)
        //CASE_CPU_OP_NII(7A, implied)
        //CASE_CPU_OP_NII(7C, absolute_x)
        //CASE_CPU_OP_NII(80, immediate)
        //CASE_CPU_OP_NII(D4, zero_page_x)
        //CASE_CPU_OP_NII(DA, implied)
        //CASE_CPU_OP_NII(DC, absolute_x)
        //CASE_CPU_OP_NII(F4, zero_page_x)
        //CASE_CPU_OP_NII(FA, implied)
        //CASE_CPU_OP_NII(FC, absolute_x)
      }
    cycles -= op_cycles;
#ifdef STATISTIC
    cpu_op_cnts[op_code] ++;
#endif
  }
  cpu_cycles += (c - cycles);
  cpu.PC = PCreg;
  cpu.A = Areg;
  cpu.X = Xreg;
  cpu.Y = Yreg;
  g_lz_zn_result = lz_zn_result;

#ifdef STATISTIC
  static int num = 0;
  if (num == 10000) {
    display_statistic();
    num = 0;
  }
  num ++;
#endif
}


// f0 - 399963 - beq
// ad - 393468 - lda
// c8 - 294519 - iny
// d0 - 267988 - bne
