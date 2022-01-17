#include "debug_defines.h"

#ifndef NEXUS_AM_TRIGGER_H
#define NEXUS_AM_TRIGGER_H

#endif //NEXUS_AM_TRIGGER_H

#define XLEN 64

// useful macros
#define READ_FIELD(x, o, l) ((x >> o) & ~((~0ULL) << l))
#define CLEAR_FIELD(x, o, l) x & (((~0ULL) << (o + l)) | ~((~0ULL) << o)))
#define SET_FIELD(x, o, l, d) x | ((d << o) &  ~((~0ULL) << (o + l)))

/*
#define TDATA1_READ(tdata1, field) READ_FIELD(tdata1, CSR_MCONTROL_ ## field ## _OFFSET, CSR_MCONTROL_ ## field ## _LENGTH)
#define TDATA1_WRITE(tdata1, field, value) SET_FIELD( \
	CLEAR_FIELD(tdata1, CSR_MCONTROL_ ## field ## _OFFSET, CSR_MCONTROL_ ## field ## _LENGTH), \
	CSR_MCONTROL_ ## field ## _OFFSET, \
	CSR_MCONTROL_ ## field ## _LENGTH, \
	value)
*/
// Simpler implementation using Spike header file
#define MCONTROL_READ(mcontrol, field) (mcontrol & CSR_MCONTROL_ ## field) >> CSR_MCONTROL_ ## field ## _OFFSET
#define MCONTROL_WRITE(mcontrol, field, data) mcontrol = (mcontrol & ~CSR_MCONTROL_ ## field) | (data << CSR_MCONTROL_ ## field ## _OFFSET)


// define some params to config testing
#define TRIGGER_EXAMINE

#define PC_SINGLE
#define INSTR_SINGLE
#define LOAD_ADDR_SINGLE
#define LOAD_PC_SINGLE
#define STORE_ADDR_SINGLE
#define STORE_ADDR_SINGLE 
#define INSTR_INSTR_CHAIN

// Trigger Listing
// #if XLEN==64
// typedef struct {
// 	union {
// 		struct {
// 			uint64_t type   : 4      ;
// 			uint64_t dmode  : 1      ;
// 			uint64_t maskmax: 6      ;
// 			uint64_t zero1  : XLEN-34;
// 			uint64_t sizehi : 2      ;
// 			uint64_t hit    : 1      ;
// 			uint64_t select : 1      ;
// 			uint64_t timing : 1      ;
// 			uint64_t sizelo : 2      ;
// 			uint64_t action : 4      ;
// 			uint64_t chain  : 1      ;
// 			uint64_t match  : 4      ;
// 			uint64_t m      : 1      ;
// 			uint64_t zero2  : 1      ;
// 			uint64_t s      : 1      ;
// 			uint64_t u      : 1      ;
// 			uint64_t execute: 1      ;
// 			uint64_t store  : 1      ;
// 			uint64_t load   : 1      ;
// 		} mcontrol;
// 		uint64_t val;
// 	} tdata1;
// } tdata1_field;
// #endif
