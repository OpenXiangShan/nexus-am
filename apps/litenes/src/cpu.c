#include <cpu.h>
#include <memory.h>
#include <ppu.h>
#include <klib.h>

CPU_STATE cpu;

uint32_t op_code;             // Current instruction code
uint32_t op_value, op_address; // Arguments for current instruction
int op_cycles;            // Additional instruction cycles used (e.g. when paging occurs)

unsigned long cpu_cycles;  // Total CPU Cycles Since Power Up (wraps)

//static void (*cpu_op_address_mode[256])();       // Array of address modes
//static void (*cpu_op_handler[256])();            // Array of instruction function pointers
//bool cpu_op_in_base_instruction_set[256]; // true if instruction is in base 6502 instruction set
//char *cpu_op_name[256];                   // Instruction names
//static int cpu_op_cycles[256];                   // CPU cycles used by instructions
static int cpu_op_cnts[256];                   // CPU cycles used by instructions

// Interrupt Addresses
//word cpu_nmi_interrupt_address();
//word cpu_reset_interrupt_address();
//word cpu_irq_interrupt_address();

// Updates Zero and Negative flags in P
//void cpu_update_zn_flags(byte value);

// If OP_TRACE, print current instruction with all registers into the console
void cpu_trace_instruction();

// CPU Adressing Modes
static inline void cpu_address_implied() { }

static inline void cpu_address_immediate() {
  op_value = instr_fetch(cpu.PC);
  cpu.PC++;
}

static inline void cpu_address_zero_page() {
  op_address = instr_fetch(cpu.PC);
  op_value = CPU_RAM[op_address];
  cpu.PC++;
}

static inline void cpu_address_zero_page_x() {
  op_address = (instr_fetch(cpu.PC) + cpu.X) & 0xFF;
  op_value = CPU_RAM[op_address];
  cpu.PC++;
}

static inline void cpu_address_zero_page_y() {
  op_address = (instr_fetch(cpu.PC) + cpu.Y) & 0xFF;
  op_value = CPU_RAM[op_address];
  cpu.PC++;
}

static inline void cpu_address_absolute() {
  op_address = instr_fetchw(cpu.PC);
  op_value = memory_readb(op_address);
  cpu.PC += 2;
}

static inline void cpu_address_absolute_jmp() {
  op_address = instr_fetchw(cpu.PC);
  if (op_address == cpu.PC - 1) {
    // spin to wait for interrupt, do not count
    cpu_op_cnts[0x4c] --;
  }
//  op_value = memory_readb(op_address);
//  cpu.PC += 2;
}

static inline void cpu_address_absolute_x() {
  op_address = (instr_fetchw(cpu.PC) + cpu.X) & 0xffff;
  op_value = memory_readb(op_address);
  cpu.PC += 2;

  if ((op_address ^ cpu.PC) >> 8) {
    op_cycles++;
  }
}

static inline void cpu_address_absolute_y() {
  op_address = (instr_fetchw(cpu.PC) + cpu.Y) & 0xFFFF;
  op_value = memory_readb(op_address);
  cpu.PC += 2;

  if ((op_address ^ cpu.PC) >> 8) {
    op_cycles++;
  }
}

static inline void cpu_address_relative() {
  op_address = instr_fetch(cpu.PC);
  cpu.PC++;
  op_address = cpu.PC + (int8_t)op_address;

  if ((op_address ^ cpu.PC) >> 8) {
    op_cycles++;
  }
}

static inline void cpu_address_indirect() {
  uint32_t arg_addr = memory_readw(cpu.PC);

  // The famous 6502 bug when instead of reading from $C0FF/$C100 it reads from $C0FF/$C000
  if ((arg_addr & 0xFF) == 0xFF) {
    // Buggy code
    op_address = (memory_readb(arg_addr & 0xFF00) << 8) | memory_readb(arg_addr);
  }
  else {
    // Normal code
    op_address = memory_readw(arg_addr);
  }
  cpu.PC += 2;
}

static inline void cpu_address_indirect_x() {
  uint32_t arg_addr = (instr_fetch(cpu.PC) + cpu.X) & 0xFF;
  //op_address = (memory_readb((arg_addr + cpu.X + 1) & 0xFF) << 8) | memory_readb((arg_addr + cpu.X) & 0xFF);
//  op_address = memory_readw((arg_addr + cpu.X) & 0xFF);
  assert(0);
  op_address = (CPU_RAM[arg_addr + 1] << 8) | CPU_RAM[arg_addr];
  op_value = memory_readb(op_address);
  cpu.PC++;
}

static inline void cpu_address_indirect_y() {
  uint32_t arg_addr = instr_fetch(cpu.PC);
  uint32_t temp = (CPU_RAM[arg_addr + 1] << 8) | CPU_RAM[arg_addr];
  op_address = (temp + (cpu.Y & 0xff)) & 0xFFFF;
  op_value = memory_readb(op_address);
  cpu.PC++;

  if ((op_address ^ cpu.PC) >> 8) {
    op_cycles++;
  }
}


// CPU Memory

byte CPU_RAM[0x8000];

inline uint32_t cpu_ram_read(uint32_t address) {
  return CPU_RAM[address & 0x7FF];
}

void cpu_ram_write(uint32_t address, uint32_t byte_data) {
  CPU_RAM[address & 0x7FF] = byte_data;
}



// Interrupt Addresses

static inline uint32_t cpu_nmi_interrupt_address()   { return memory_readw(0xFFFA); }
static inline uint32_t cpu_reset_interrupt_address() { return memory_readw(0xFFFC); }
static inline uint32_t cpu_irq_interrupt_address()   { return memory_readw(0xFFFE); }



// Stack Routines

static inline void cpu_stack_pushb(uint32_t byte_data) { memory_writeb(0x100 + cpu.SP--, byte_data);       }
static inline void cpu_stack_pushw(uint32_t word_data) { memory_writew(0xFF + cpu.SP, word_data); cpu.SP -= 2; }
static inline uint32_t cpu_stack_popb()           { return memory_readb(0x100 + ++cpu.SP);       }
static inline uint32_t cpu_stack_popw()           { cpu.SP += 2; return memory_readw(0xFF + cpu.SP); }



// CPU Instructions

static inline void ____FE____() { /* Instruction for future Extension */ }

#define cpu_flag_set(flag) cpu.P[flag]
#define cpu_modify_flag(flag, value) cpu.P[flag] = !!(value)
#define cpu_set_flag(flag) cpu.P[flag] = 1
#define cpu_unset_flag(flag) cpu.P[flag] = 0

static inline void cpu_update_zn_flags(uint32_t value) {
  cpu_modify_flag(zero_bp, (value & 0xff) == 0);
  cpu_modify_flag(negative_bp, (value >> 7) & 1);
}

#define cpu_branch(flag) do { \
	if (flag) { \
		cpu.PC = op_address; \
	} \
} while(0)
#define cpu_compare(reg) int result = (reg & 0xff) - op_value; \
                         cpu_modify_flag(carry_bp, result >= 0); \
                         cpu_modify_flag(zero_bp, result == 0); \
                         cpu_modify_flag(negative_bp, (result >> 7) & 1);



// CPU Instructions

// NOP

static inline void cpu_op_nop() { }

// Addition

static inline void cpu_op_adc()
{
    int result = (cpu.A & 0xff) + op_value + cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, (result & 0x100));
    cpu_modify_flag(overflow_bp, (~(cpu.A ^ op_value) & (cpu.A ^ result) & 0x80));
    cpu_update_zn_flags(result);
    cpu.A = result;
}

// Subtraction

static inline void cpu_op_sbc()
{
    int result = (cpu.A & 0xff) - op_value - !cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, !(result & 0x100));
    cpu_modify_flag(overflow_bp, ((cpu.A ^ op_value) & (cpu.A ^ result) & 0x80));
    cpu_update_zn_flags(result);
    cpu.A = result;
}

// Bit Manipulation Operations

static inline void cpu_op_and() { cpu_update_zn_flags(cpu.A &= op_value); }
static inline void cpu_op_bit() {
  cpu_modify_flag(zero_bp, !(cpu.A & op_value & 0xff));
  cpu_modify_flag(6, (op_value >> 6) & 0x1);
  cpu_modify_flag(7, (op_value >> 7) & 0x1);
}
static inline void cpu_op_eor() { cpu_update_zn_flags(cpu.A ^= op_value); }
static inline void cpu_op_ora() { cpu_update_zn_flags(cpu.A |= op_value); }
static inline void cpu_op_asla()
{
    cpu_modify_flag(carry_bp, cpu.A & 0x80);
    cpu.A <<= 1;
    cpu_update_zn_flags(cpu.A);
}
static inline void cpu_op_asl()
{
    cpu_modify_flag(carry_bp, op_value & 0x80);
    op_value <<= 1;
    op_value &= 0xFF;
    cpu_update_zn_flags(op_value);
    memory_writeb(op_address, op_value);
}
static inline void cpu_op_lsra()
{
    int value = (cpu.A & 0xff) >> 1;
    cpu_modify_flag(carry_bp, cpu.A & 0x01);
    cpu_update_zn_flags(value);
    cpu.A = value;
}
static inline void cpu_op_lsr()
{
    cpu_modify_flag(carry_bp, op_value & 0x01);
    op_value >>= 1;
    op_value &= 0xFF;
    memory_writeb(op_address, op_value);
    cpu_update_zn_flags(op_value);
}

static inline void cpu_op_rola()
{
    int value = (cpu.A & 0xff) << 1;
    value |= cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, value > 0xFF);
    cpu.A = value & 0xFF;
    cpu_update_zn_flags(cpu.A);
}
static inline void cpu_op_rol()
{
    op_value <<= 1;
    op_value |= cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, op_value > 0xFF);
    op_value &= 0xFF;
    memory_writeb(op_address, op_value);
    cpu_update_zn_flags(op_value);
}
static inline void cpu_op_rora()
{
    unsigned char carry = cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, cpu.A & 0x01);
    cpu.A = ((cpu.A & 0xff) >> 1) | (carry << 7);
    cpu_modify_flag(zero_bp, (cpu.A & 0xff) == 0);
    cpu_modify_flag(negative_bp, carry);
}
static inline void cpu_op_ror()
{
    unsigned char carry = cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, op_value & 0x01);
    op_value = ((op_value >> 1) | (carry << 7)) & 0xFF;
    cpu_modify_flag(zero_bp, op_value == 0);
    cpu_modify_flag(negative_bp, carry);
    memory_writeb(op_address, op_value);
}

// Loading

static inline void cpu_op_lda() { cpu_update_zn_flags(cpu.A = op_value); }
static inline void cpu_op_ldx() { cpu_update_zn_flags(cpu.X = op_value); }
static inline void cpu_op_ldy() { cpu_update_zn_flags(cpu.Y = op_value); }

// Storing

static inline void cpu_op_sta() { memory_writeb(op_address, cpu.A & 0xff); }
static inline void cpu_op_stx() { memory_writeb(op_address, cpu.X & 0xff); }
static inline void cpu_op_sty() { memory_writeb(op_address, cpu.Y & 0xff); }

// Transfering

static inline void cpu_op_tax() { cpu_update_zn_flags(cpu.X = cpu.A);  }
static inline void cpu_op_txa() { cpu_update_zn_flags(cpu.A = cpu.X);  }
static inline void cpu_op_tay() { cpu_update_zn_flags(cpu.Y = cpu.A);  }
static inline void cpu_op_tya() { cpu_update_zn_flags(cpu.A = cpu.Y);  }
static inline void cpu_op_tsx() { cpu_update_zn_flags(cpu.X = cpu.SP); }
static inline void cpu_op_txs() { cpu.SP = cpu.X & 0xff; }

// Branching Positive

static inline void cpu_op_bcs() { cpu_branch(cpu_flag_set(carry_bp));     }
static inline void cpu_op_beq() { cpu_branch(cpu_flag_set(zero_bp));      }
static inline void cpu_op_bmi() { cpu_branch(cpu_flag_set(negative_bp));  }
static inline void cpu_op_bvs() { cpu_branch(cpu_flag_set(overflow_bp));  }

// Branching Negative

static inline void cpu_op_bne() { cpu_branch(!cpu_flag_set(zero_bp));     }
static inline void cpu_op_bcc() { cpu_branch(!cpu_flag_set(carry_bp));    }
static inline void cpu_op_bpl() { cpu_branch(!cpu_flag_set(negative_bp)); }
static inline void cpu_op_bvc() { cpu_branch(!cpu_flag_set(overflow_bp)); }

// Jumping

static inline void cpu_op_jmp() { cpu.PC = op_address; }

// Subroutines

static inline void cpu_op_jsr() { cpu_stack_pushw(cpu.PC - 1); cpu.PC = op_address; }
static inline void cpu_op_rts() { cpu.PC = cpu_stack_popw() + 1; }

// Interruptions

static inline void cpu_op_brk() {
  cpu_stack_pushw(cpu.PC - 1);
  cpu_stack_pushb(byte_pack(cpu.P));
  cpu.P[unused_bp] = 1;
  cpu.P[break_bp] = 1;
  cpu.PC = cpu_nmi_interrupt_address();
}
static inline void cpu_op_rti() {
  byte_unpack(cpu.P, cpu_stack_popb());
  cpu.P[unused_bp] = 1;
	cpu.PC = cpu_stack_popw();
}

// Flags

static inline void cpu_op_clc() { cpu_unset_flag(carry_bp);     }
static inline void cpu_op_cld() { cpu_unset_flag(decimal_bp);   }
static inline void cpu_op_cli() { cpu_unset_flag(interrupt_bp); }
static inline void cpu_op_clv() { cpu_unset_flag(overflow_bp);  }
static inline void cpu_op_sec() { cpu_set_flag(carry_bp);       }
static inline void cpu_op_sed() { cpu_set_flag(decimal_bp);     }
static inline void cpu_op_sei() { cpu_set_flag(interrupt_bp);   }

// Comparison

static inline void cpu_op_cmp() { cpu_compare(cpu.A & 0xff); }
static inline void cpu_op_cpx() { cpu_compare(cpu.X & 0xff); }
static inline void cpu_op_cpy() { cpu_compare(cpu.Y & 0xff); }

// Increment

static inline void cpu_op_inc() { uint32_t result = op_value + 1; memory_writeb(op_address, result); cpu_update_zn_flags(result); }
static inline void cpu_op_inx() { cpu.X = (cpu.X + 1) & 0xff; cpu_update_zn_flags(cpu.X); }
static inline void cpu_op_iny() { cpu.Y = (cpu.Y + 1) & 0xff; cpu_update_zn_flags(cpu.Y); }

// Decrement

static inline void cpu_op_dec() { uint32_t result = op_value - 1; memory_writeb(op_address, result); cpu_update_zn_flags(result); }
static inline void cpu_op_dex() { cpu.X = (cpu.X - 1) & 0xff; cpu_update_zn_flags(cpu.X); }
static inline void cpu_op_dey() { cpu.Y = (cpu.Y - 1) & 0xff; cpu_update_zn_flags(cpu.Y); }

// Stack

static inline void cpu_op_php() { cpu_stack_pushb(byte_pack(cpu.P) | 0x30); }
static inline void cpu_op_pha() { cpu_stack_pushb(cpu.A & 0xff); }
static inline void cpu_op_pla() { cpu.A = cpu_stack_popb(); cpu_update_zn_flags(cpu.A); }
static inline void cpu_op_plp() {
//  cpu.P = (cpu_stack_popb() & 0xEF) | 0x20;
  byte_unpack(cpu.P, cpu_stack_popb());
  cpu.P[break_bp] = 0;
  cpu.P[unused_bp] = 1;
}



// Extended Instruction Set

static inline void cpu_op_aso() { cpu_op_asl(); cpu_op_ora(); }
static inline void cpu_op_axa() { memory_writeb(op_address, cpu.A & (cpu.X & 0xff) & (op_address >> 8)); }
static inline void cpu_op_axs() { memory_writeb(op_address, cpu.A & (cpu.X & 0xff)); }
static inline void cpu_op_dcm()
{
    op_value--;
    op_value &= 0xFF;
    memory_writeb(op_address, op_value);
    cpu_op_cmp();
}
static inline void cpu_op_ins()
{
    op_value = (op_value + 1) & 0xFF;
    memory_writeb(op_address, op_value);
    cpu_op_sbc();
}
static inline void cpu_op_lax() { cpu_update_zn_flags(cpu.A = cpu.X = op_value & 0xff); }
static inline void cpu_op_lse() { cpu_op_lsr(); cpu_op_eor(); }
static inline void cpu_op_rla() { cpu_op_rol(); cpu_op_and(); }
static inline void cpu_op_rra() { cpu_op_ror(); cpu_op_adc(); }





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


#define CASE_CPU_OP_BIS(o, c, f, n, a) \
  case 0x##o: __cycles = c; \
              cpu_address_##a(); \
              cpu_op_##f(); \
              cpu_op_cnts[0x##o] ++; \
              break;

#define CASE_CPU_OP_NII(o, a) \
  case 0x##o: __cycles = 1; \
              cpu_address_##a(); \
              ____FE____(); \
              cpu_op_cnts[0x##o] ++; \
              break;

#define CASE_CPU_OP_EIS(o, c, f, n, a) CASE_CPU_OP_BIS(o, c, f, n, a)


// CPU Lifecycle

void cpu_init()
{
  /*
    CPU_OP_BIS(00, 7, brk, "BRK", implied)
    CPU_OP_BIS(01, 6, ora, "ORA", indirect_x)
    CPU_OP_BIS(05, 3, ora, "ORA", zero_page)
    CPU_OP_BIS(06, 5, asl, "ASL", zero_page)
    CPU_OP_BIS(08, 3, php, "PHP", implied)
    CPU_OP_BIS(09, 2, ora, "ORA", immediate)
    CPU_OP_BIS(0A, 2, asla,"ASL", implied)
    CPU_OP_BIS(0D, 4, ora, "ORA", absolute)
    CPU_OP_BIS(0E, 6, asl, "ASL", absolute)
    CPU_OP_BIS(10, 2, bpl, "BPL", relative)
    CPU_OP_BIS(11, 5, ora, "ORA", indirect_y)
    CPU_OP_BIS(15, 4, ora, "ORA", zero_page_x)
    CPU_OP_BIS(16, 6, asl, "ASL", zero_page_x)
    CPU_OP_BIS(18, 2, clc, "CLC", implied)
    CPU_OP_BIS(19, 4, ora, "ORA", absolute_y)
    CPU_OP_BIS(1D, 4, ora, "ORA", absolute_x)
    CPU_OP_BIS(1E, 7, asl, "ASL", absolute_x)
    CPU_OP_BIS(20, 6, jsr, "JSR", absolute)
    CPU_OP_BIS(21, 6, and, "AND", indirect_x)
    CPU_OP_BIS(24, 3, bit, "BIT", zero_page)
    CPU_OP_BIS(25, 3, and, "AND", zero_page)
    CPU_OP_BIS(26, 5, rol, "ROL", zero_page)
    CPU_OP_BIS(28, 4, plp, "PLP", implied)
    CPU_OP_BIS(29, 2, and, "AND", immediate)
    CPU_OP_BIS(2A, 2, rola,"ROL", implied)
    CPU_OP_BIS(2C, 4, bit, "BIT", absolute)
    CPU_OP_BIS(2D, 2, and, "AND", absolute)
    CPU_OP_BIS(2E, 6, rol, "ROL", absolute)
    CPU_OP_BIS(30, 2, bmi, "BMI", relative)
    CPU_OP_BIS(31, 5, and, "AND", indirect_y)
    CPU_OP_BIS(35, 4, and, "AND", zero_page_x)
    CPU_OP_BIS(36, 6, rol, "ROL", zero_page_x)
    CPU_OP_BIS(38, 2, sec, "SEC", implied)
    CPU_OP_BIS(39, 4, and, "AND", absolute_y)
    CPU_OP_BIS(3D, 4, and, "AND", absolute_x)
    CPU_OP_BIS(3E, 7, rol, "ROL", absolute_x)
    CPU_OP_BIS(40, 6, rti, "RTI", implied)
    CPU_OP_BIS(41, 6, eor, "EOR", indirect_x)
    CPU_OP_BIS(45, 3, eor, "EOR", zero_page)
    CPU_OP_BIS(46, 5, lsr, "LSR", zero_page)
    CPU_OP_BIS(48, 3, pha, "PHA", implied)
    CPU_OP_BIS(49, 2, eor, "EOR", immediate)
    CPU_OP_BIS(4A, 2, lsra,"LSR", implied)
    CPU_OP_BIS(4C, 3, jmp, "JMP", absolute)
    CPU_OP_BIS(4D, 4, eor, "EOR", absolute)
    CPU_OP_BIS(4E, 6, lsr, "LSR", absolute)
    CPU_OP_BIS(50, 2, bvc, "BVC", relative)
    CPU_OP_BIS(51, 5, eor, "EOR", indirect_y)
    CPU_OP_BIS(55, 4, eor, "EOR", zero_page_x)
    CPU_OP_BIS(56, 6, lsr, "LSR", zero_page_x)
    CPU_OP_BIS(58, 2, cli, "CLI", implied)
    CPU_OP_BIS(59, 4, eor, "EOR", absolute_y)
    CPU_OP_BIS(5D, 4, eor, "EOR", absolute_x)
    CPU_OP_BIS(5E, 7, lsr, "LSR", absolute_x)
    CPU_OP_BIS(60, 6, rts, "RTS", implied)
    CPU_OP_BIS(61, 6, adc, "ADC", indirect_x)
    CPU_OP_BIS(65, 3, adc, "ADC", zero_page)
    CPU_OP_BIS(66, 5, ror, "ROR", zero_page)
    CPU_OP_BIS(68, 4, pla, "PLA", implied)
    CPU_OP_BIS(69, 2, adc, "ADC", immediate)
    CPU_OP_BIS(6A, 2, rora,"ROR", implied)
    CPU_OP_BIS(6C, 5, jmp, "JMP", indirect)
    CPU_OP_BIS(6D, 4, adc, "ADC", absolute)
    CPU_OP_BIS(6E, 6, ror, "ROR", absolute)
    CPU_OP_BIS(70, 2, bvs, "BVS", relative)
    CPU_OP_BIS(71, 5, adc, "ADC", indirect_y)
    CPU_OP_BIS(75, 4, adc, "ADC", zero_page_x)
    CPU_OP_BIS(76, 6, ror, "ROR", zero_page_x)
    CPU_OP_BIS(78, 2, sei, "SEI", implied)
    CPU_OP_BIS(79, 4, adc, "ADC", absolute_y)
    CPU_OP_BIS(7D, 4, adc, "ADC", absolute_x)
    CPU_OP_BIS(7E, 7, ror, "ROR", absolute_x)
    CPU_OP_BIS(81, 6, sta, "STA", indirect_x)
    CPU_OP_BIS(84, 3, sty, "STY", zero_page)
    CPU_OP_BIS(85, 3, sta, "STA", zero_page)
    CPU_OP_BIS(86, 3, stx, "STX", zero_page)
    CPU_OP_BIS(88, 2, dey, "DEY", implied)
    CPU_OP_BIS(8A, 2, txa, "TXA", implied)
    CPU_OP_BIS(8C, 4, sty, "STY", absolute)
    CPU_OP_BIS(8D, 4, sta, "STA", absolute)
    CPU_OP_BIS(8E, 4, stx, "STX", absolute)
    CPU_OP_BIS(90, 2, bcc, "BCC", relative)
    CPU_OP_BIS(91, 6, sta, "STA", indirect_y)
    CPU_OP_BIS(94, 4, sty, "STY", zero_page_x)
    CPU_OP_BIS(95, 4, sta, "STA", zero_page_x)
    CPU_OP_BIS(96, 4, stx, "STX", zero_page_y)
    CPU_OP_BIS(98, 2, tya, "TYA", implied)
    CPU_OP_BIS(99, 5, sta, "STA", absolute_y)
    CPU_OP_BIS(9A, 2, txs, "TXS", implied)
    CPU_OP_BIS(9D, 5, sta, "STA", absolute_x)
    CPU_OP_BIS(A0, 2, ldy, "LDY", immediate)
    CPU_OP_BIS(A1, 6, lda, "LDA", indirect_x)
    CPU_OP_BIS(A2, 2, ldx, "LDX", immediate)
    CPU_OP_BIS(A4, 3, ldy, "LDY", zero_page)
    CPU_OP_BIS(A5, 3, lda, "LDA", zero_page)
    CPU_OP_BIS(A6, 3, ldx, "LDX", zero_page)
    CPU_OP_BIS(A8, 2, tay, "TAY", implied)
    CPU_OP_BIS(A9, 2, lda, "LDA", immediate)
    CPU_OP_BIS(AA, 2, tax, "TAX", implied)
    CPU_OP_BIS(AC, 4, ldy, "LDY", absolute)
    CPU_OP_BIS(AD, 4, lda, "LDA", absolute)
    CPU_OP_BIS(AE, 4, ldx, "LDX", absolute)
    CPU_OP_BIS(B0, 2, bcs, "BCS", relative)
    CPU_OP_BIS(B1, 5, lda, "LDA", indirect_y)
    CPU_OP_BIS(B4, 4, ldy, "LDY", zero_page_x)
    CPU_OP_BIS(B5, 4, lda, "LDA", zero_page_x)
    CPU_OP_BIS(B6, 4, ldx, "LDX", zero_page_y)
    CPU_OP_BIS(B8, 2, clv, "CLV", implied)
    CPU_OP_BIS(B9, 4, lda, "LDA", absolute_y)
    CPU_OP_BIS(BA, 2, tsx, "TSX", implied)
    CPU_OP_BIS(BC, 4, ldy, "LDY", absolute_x)
    CPU_OP_BIS(BD, 4, lda, "LDA", absolute_x)
    CPU_OP_BIS(BE, 4, ldx, "LDX", absolute_y)
    CPU_OP_BIS(C0, 2, cpy, "CPY", immediate)
    CPU_OP_BIS(C1, 6, cmp, "CMP", indirect_x)
    CPU_OP_BIS(C4, 3, cpy, "CPY", zero_page)
    CPU_OP_BIS(C5, 3, cmp, "CMP", zero_page)
    CPU_OP_BIS(C6, 5, dec, "DEC", zero_page)
    CPU_OP_BIS(C8, 2, iny, "INY", implied)
    CPU_OP_BIS(C9, 2, cmp, "CMP", immediate)
    CPU_OP_BIS(CA, 2, dex, "DEX", implied)
    CPU_OP_BIS(CC, 4, cpy, "CPY", absolute)
    CPU_OP_BIS(CD, 4, cmp, "CMP", absolute)
    CPU_OP_BIS(CE, 6, dec, "DEC", absolute)
    CPU_OP_BIS(D0, 2, bne, "BNE", relative)
    CPU_OP_BIS(D1, 5, cmp, "CMP", indirect_y)
    CPU_OP_BIS(D5, 4, cmp, "CMP", zero_page_x)
    CPU_OP_BIS(D6, 6, dec, "DEC", zero_page_x)
    CPU_OP_BIS(D8, 2, cld, "CLD", implied)
    CPU_OP_BIS(D9, 4, cmp, "CMP", absolute_y)
    CPU_OP_BIS(DD, 4, cmp, "CMP", absolute_x)
    CPU_OP_BIS(DE, 7, dec, "DEC", absolute_x)
    CPU_OP_BIS(E0, 2, cpx, "CPX", immediate)
    CPU_OP_BIS(E1, 6, sbc, "SBC", indirect_x)
    CPU_OP_BIS(E4, 3, cpx, "CPX", zero_page)
    CPU_OP_BIS(E5, 3, sbc, "SBC", zero_page)
    CPU_OP_BIS(E6, 5, inc, "INC", zero_page)
    CPU_OP_BIS(E8, 2, inx, "INX", implied)
    CPU_OP_BIS(E9, 2, sbc, "SBC", immediate)
    CPU_OP_BIS(EA, 2, nop, "NOP", implied)
    CPU_OP_BIS(EC, 4, cpx, "CPX", absolute)
    CPU_OP_BIS(ED, 4, sbc, "SBC", absolute)
    CPU_OP_BIS(EE, 6, inc, "INC", absolute)
    CPU_OP_BIS(F0, 2, beq, "BEQ", relative)
    CPU_OP_BIS(F1, 5, sbc, "SBC", indirect_y)
    CPU_OP_BIS(F5, 4, sbc, "SBC", zero_page_x)
    CPU_OP_BIS(F6, 6, inc, "INC", zero_page_x)
    CPU_OP_BIS(F8, 2, sed, "SED", implied)
    CPU_OP_BIS(F9, 4, sbc, "SBC", absolute_y)
    CPU_OP_BIS(FD, 4, sbc, "SBC", absolute_x)
    CPU_OP_BIS(FE, 7, inc, "INC", absolute_x)

    CPU_OP_EIS(03, 8, aso, "SLO", indirect_x)
    CPU_OP_EIS(07, 5, aso, "SLO", zero_page)
    CPU_OP_EIS(0F, 6, aso, "SLO", absolute)
    CPU_OP_EIS(13, 8, aso, "SLO", indirect_y)
    CPU_OP_EIS(17, 6, aso, "SLO", zero_page_x)
    CPU_OP_EIS(1B, 7, aso, "SLO", absolute_y)
    CPU_OP_EIS(1F, 7, aso, "SLO", absolute_x)
    CPU_OP_EIS(23, 8, rla, "RLA", indirect_x)
    CPU_OP_EIS(27, 5, rla, "RLA", zero_page)
    CPU_OP_EIS(2F, 6, rla, "RLA", absolute)
    CPU_OP_EIS(33, 8, rla, "RLA", indirect_y)
    CPU_OP_EIS(37, 6, rla, "RLA", zero_page_x)
    CPU_OP_EIS(3B, 7, rla, "RLA", absolute_y)
    CPU_OP_EIS(3F, 7, rla, "RLA", absolute_x)
    CPU_OP_EIS(43, 8, lse, "SRE", indirect_x)
    CPU_OP_EIS(47, 5, lse, "SRE", zero_page)
    CPU_OP_EIS(4F, 6, lse, "SRE", absolute)
    CPU_OP_EIS(53, 8, lse, "SRE", indirect_y)
    CPU_OP_EIS(57, 6, lse, "SRE", zero_page_x)
    CPU_OP_EIS(5B, 7, lse, "SRE", absolute_y)
    CPU_OP_EIS(5F, 7, lse, "SRE", absolute_x)
    CPU_OP_EIS(63, 8, rra, "RRA", indirect_x)
    CPU_OP_EIS(67, 5, rra, "RRA", zero_page)
    CPU_OP_EIS(6F, 6, rra, "RRA", absolute)
    CPU_OP_EIS(73, 8, rra, "RRA", indirect_y)
    CPU_OP_EIS(77, 6, rra, "RRA", zero_page_x)
    CPU_OP_EIS(7B, 7, rra, "RRA", absolute_y)
    CPU_OP_EIS(7F, 7, rra, "RRA", absolute_x)
    CPU_OP_EIS(83, 6, axs, "SAX", indirect_x)
    CPU_OP_EIS(87, 3, axs, "SAX", zero_page)
    CPU_OP_EIS(8F, 4, axs, "SAX", absolute)
    CPU_OP_EIS(93, 6, axa, "SAX", indirect_y)
    CPU_OP_EIS(97, 4, axs, "SAX", zero_page_y)
    CPU_OP_EIS(9F, 5, axa, "SAX", absolute_y)
    CPU_OP_EIS(A3, 6, lax, "LAX", indirect_x)
    CPU_OP_EIS(A7, 3, lax, "LAX", zero_page)
    CPU_OP_EIS(AF, 4, lax, "LAX", absolute)
    CPU_OP_EIS(B3, 5, lax, "LAX", indirect_y)
    CPU_OP_EIS(B7, 4, lax, "LAX", zero_page_y)
    CPU_OP_EIS(BF, 4, lax, "LAX", absolute_y)
    CPU_OP_EIS(C3, 8, dcm, "DCP", indirect_x)
    CPU_OP_EIS(C7, 5, dcm, "DCP", zero_page)
    CPU_OP_EIS(CF, 6, dcm, "DCP", absolute)
    CPU_OP_EIS(D3, 8, dcm, "DCP", indirect_y)
    CPU_OP_EIS(D7, 6, dcm, "DCP", zero_page_x)
    CPU_OP_EIS(DB, 7, dcm, "DCP", absolute_y)
    CPU_OP_EIS(DF, 7, dcm, "DCP", absolute_x)
    CPU_OP_EIS(E3, 8, ins, "ISB", indirect_x)
    CPU_OP_EIS(E7, 5, ins, "ISB", zero_page)
    CPU_OP_EIS(EB, 2, sbc, "SBC", immediate)
    CPU_OP_EIS(EF, 6, ins, "ISB", absolute)
    CPU_OP_EIS(F3, 8, ins, "ISB", indirect_y)
    CPU_OP_EIS(F7, 6, ins, "ISB", zero_page_x)
    CPU_OP_EIS(FB, 7, ins, "ISB", absolute_y)
    CPU_OP_EIS(FF, 7, ins, "ISB", absolute_x)

    CPU_OP_NII(04, zero_page)
    CPU_OP_NII(0C, absolute)
    CPU_OP_NII(14, zero_page_x)
    CPU_OP_NII(1A, implied)
    CPU_OP_NII(1C, absolute_x)
    CPU_OP_NII(34, zero_page_x)
    CPU_OP_NII(3A, implied)
    CPU_OP_NII(3C, absolute_x)
    CPU_OP_NII(44, zero_page)
    CPU_OP_NII(54, zero_page_x)
    CPU_OP_NII(5A, implied)
    CPU_OP_NII(5C, absolute_x)
    CPU_OP_NII(64, zero_page)
    CPU_OP_NII(74, zero_page_x)
    CPU_OP_NII(7A, implied)
    CPU_OP_NII(7C, absolute_x)
    CPU_OP_NII(80, immediate)
    CPU_OP_NII(D4, zero_page_x)
    CPU_OP_NII(DA, implied)
    CPU_OP_NII(DC, absolute_x)
    CPU_OP_NII(F4, zero_page_x)
    CPU_OP_NII(FA, implied)
    CPU_OP_NII(FC, absolute_x)
    */

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

void cpu_interrupt()
{
    // if (ppu_in_vblank()) {
        if (ppu_generates_nmi()) {
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
  int __cycles;
  while (cycles > 0) {
    op_code = instr_fetch(cpu.PC++); //memory_readb(cpu.PC++);
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
        CASE_CPU_OP_BIS(20, 6, jsr, "JSR", absolute)
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
        CASE_CPU_OP_BIS(81, 6, sta, "STA", indirect_x)
        CASE_CPU_OP_BIS(84, 3, sty, "STY", zero_page)
        CASE_CPU_OP_BIS(85, 3, sta, "STA", zero_page)
        CASE_CPU_OP_BIS(86, 3, stx, "STX", zero_page)
        CASE_CPU_OP_BIS(88, 2, dey, "DEY", implied)
        CASE_CPU_OP_BIS(8A, 2, txa, "TXA", implied)
        CASE_CPU_OP_BIS(8C, 4, sty, "STY", absolute)
        CASE_CPU_OP_BIS(8D, 4, sta, "STA", absolute)
        CASE_CPU_OP_BIS(8E, 4, stx, "STX", absolute)
        CASE_CPU_OP_BIS(90, 2, bcc, "BCC", relative)
        CASE_CPU_OP_BIS(91, 6, sta, "STA", indirect_y)
        CASE_CPU_OP_BIS(94, 4, sty, "STY", zero_page_x)
        CASE_CPU_OP_BIS(95, 4, sta, "STA", zero_page_x)
        CASE_CPU_OP_BIS(96, 4, stx, "STX", zero_page_y)
        CASE_CPU_OP_BIS(98, 2, tya, "TYA", implied)
        CASE_CPU_OP_BIS(99, 5, sta, "STA", absolute_y)
        CASE_CPU_OP_BIS(9A, 2, txs, "TXS", implied)
        CASE_CPU_OP_BIS(9D, 5, sta, "STA", absolute_x)
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
        CASE_CPU_OP_EIS(83, 6, axs, "SAX", indirect_x)
        CASE_CPU_OP_EIS(87, 3, axs, "SAX", zero_page)
        CASE_CPU_OP_EIS(8F, 4, axs, "SAX", absolute)
        CASE_CPU_OP_EIS(93, 6, axa, "SAX", indirect_y)
        CASE_CPU_OP_EIS(97, 4, axs, "SAX", zero_page_y)
        CASE_CPU_OP_EIS(9F, 5, axa, "SAX", absolute_y)
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

        CASE_CPU_OP_NII(04, zero_page)
        CASE_CPU_OP_NII(0C, absolute)
        CASE_CPU_OP_NII(14, zero_page_x)
        CASE_CPU_OP_NII(1A, implied)
        CASE_CPU_OP_NII(1C, absolute_x)
        CASE_CPU_OP_NII(34, zero_page_x)
        CASE_CPU_OP_NII(3A, implied)
        CASE_CPU_OP_NII(3C, absolute_x)
        CASE_CPU_OP_NII(44, zero_page)
        CASE_CPU_OP_NII(54, zero_page_x)
        CASE_CPU_OP_NII(5A, implied)
        CASE_CPU_OP_NII(5C, absolute_x)
        CASE_CPU_OP_NII(64, zero_page)
        CASE_CPU_OP_NII(74, zero_page_x)
        CASE_CPU_OP_NII(7A, implied)
        CASE_CPU_OP_NII(7C, absolute_x)
        CASE_CPU_OP_NII(80, immediate)
        CASE_CPU_OP_NII(D4, zero_page_x)
        CASE_CPU_OP_NII(DA, implied)
        CASE_CPU_OP_NII(DC, absolute_x)
        CASE_CPU_OP_NII(F4, zero_page_x)
        CASE_CPU_OP_NII(FA, implied)
        CASE_CPU_OP_NII(FC, absolute_x)
      }
    cycles -= __cycles + op_cycles;
    cpu_cycles -= __cycles + op_cycles;
    //cycles -= cpu_op_cycles[op_code] + op_cycles;
    //cpu_cycles -= cpu_op_cycles[op_code] + op_cycles;
    op_cycles = 0;
  }

  static int num = 0;
  if (num == 10000) {
    display_statistic();
    num = 0;
  }
  num ++;
}


// f0 - 399963 - beq
// ad - 393468 - lda
// c8 - 294519 - iny
// d0 - 267988 - bne
