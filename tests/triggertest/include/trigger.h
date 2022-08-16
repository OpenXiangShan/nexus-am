#include "debug_defines.h"
#include "encoding.h"

#ifndef NEXUS_AM_TRIGGER_H
#define NEXUS_AM_TRIGGER_H

#endif //NEXUS_AM_TRIGGER_H

#define XLEN 64

// useful macros
// #define READ_FIELD(x, o, l) ((x >> o) & ~((~0ULL) << l))
// #define CLEAR_FIELD(x, o, l) x & (((~0ULL) << (o + l)) | ~((~0ULL) << o)))
// #define SET_FIELD(x, o, l, d) x | ((d << o) &  ~((~0ULL) << (o + l)))

/*
#define TDATA1_READ(tdata1, field) READ_FIELD(tdata1, CSR_MCONTROL_ ## field ## _OFFSET, CSR_MCONTROL_ ## field ## _LENGTH)
#define TDATA1_WRITE(tdata1, field, value) SET_FIELD( \
	CLEAR_FIELD(tdata1, CSR_MCONTROL_ ## field ## _OFFSET, CSR_MCONTROL_ ## field ## _LENGTH), \
	CSR_MCONTROL_ ## field ## _OFFSET, \
	CSR_MCONTROL_ ## field ## _LENGTH, \
	value)
*/
// Simpler implementation using Spike header file
#define READ_FIELD(data, csr, field) \
  (data & CSR_ ## csr ## _ ## field) >> \
	CSR_ ## csr ## _ ## field ## _OFFSET
#define GEN_FIELD(csr, field, value) \
  ((value << CSR_ ## csr ## _ ## field ## _OFFSET) & \
	CSR_ ## csr ## _ ## field)

#define GEN_MCONTROL(field, value) GEN_FIELD(MCONTROL, field, value)
#define READ_MCONTROL(data, field) READ_FIELD(data, MCONTROL, field)

#define MCONTROL_READ(mcontrol, field) (mcontrol & CSR_MCONTROL_ ## field) >> CSR_MCONTROL_ ## field ## _OFFSET
#define MCONTROL_WRITE(mcontrol, field, data) mcontrol = (mcontrol & ~CSR_MCONTROL_ ## field) | (data << CSR_MCONTROL_ ## field ## _OFFSET)


// define some params to config testing
// #define TRIGGER_EXAMINE

// #define PC_SINGLE
// #define INSTR_SINGLE
// #define LOAD_ADDR_SINGLE
// #define LOAD_PC_SINGLE
// #define STORE_ADDR_SINGLE
// #define STORE_ADDR_SINGLE 
// #define INSTR_INSTR_CHAIN

#define TRIGGER_WRITE(t_name, t_type, t_select) \
void t_name ## _trigger(unsigned int select, uintptr_t addr, uint8_t chain, uint8_t match) { \
  write_csr(tselect, select); \
  uintptr_t mcontrol = 0; \
  mcontrol |= GEN_MCONTROL(t_type, 1) |\
  GEN_MCONTROL(SELECT, t_select) |\
  GEN_MCONTROL(SIZEHI, 1) |\
  GEN_MCONTROL(SIZELO, 1) |\
  GEN_MCONTROL(MATCH, match) |\
  GEN_MCONTROL(CHAIN, chain) |\
  GEN_MCONTROL(M, 1) |\
  GEN_MCONTROL(S, 1) |\
  GEN_MCONTROL(U, 1) ; \
  printf("mcontrol_to_write is %lx\n", mcontrol); \
  write_csr(tdata1, mcontrol); \
  write_csr(tdata2, addr); \
  asm __volatile__("fence.i"); \
}
