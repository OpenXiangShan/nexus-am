#include "common.h"

#ifndef CPU_H
#define CPU_H

uint32_t cpu_ram_read(uint32_t address);
void cpu_ram_write(uint32_t address, uint32_t byte_data);

void cpu_init();
void cpu_reset();
void cpu_interrupt();
void cpu_run(long cycles);

// CPU cycles that passed since power up
unsigned long cpu_clock();

typedef enum {
    carry_flag     = 0x01,
    zero_flag      = 0x02,
    interrupt_flag = 0x04,
    decimal_flag   = 0x08,
    break_flag     = 0x10,
    unused_flag    = 0x20,
    overflow_flag  = 0x40,
    negative_flag  = 0x80
} cpu_p_flag;

typedef enum {
    carry_bp      = 0,
    zero_bp       = 1,
    interrupt_bp  = 2,
    decimal_bp    = 3,
    break_bp      = 4,
    unused_bp     = 5,
    overflow_bp   = 6,
    negative_bp   = 7
} cpu_p_bp;

typedef struct {
    uint32_t PC; // Program Counter,
    uint32_t SP; // Stack Pointer,
    uint32_t A, X, Y; // Registers
    //byte P; // Flag Register
    int P[8]; // Expended Flag Register
} CPU_STATE;

#endif
