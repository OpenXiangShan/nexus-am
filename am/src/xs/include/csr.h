#ifndef __CSR_H__
#define __CSR_H__

// csr addr const

// pmp csr
#define PMPCFG_BASE 0x3a0
#define PMPADDR_BASE 0x3b0
#define PMP_R 1
#define PMP_W 2
#define PMP_X 4

#define PMP_A_OFF 0
#define PMP_A_TOR 1
#define PMP_A_NA4 2
#define PMP_A_NAPOT 3

// currently XiangShan only support 16 PMP entries
#define PMP_COUNT 16

#define CSR_PMPCFG0 0x3a0
#define CSR_PMPADDR0 0x3b0

// hpm csr
#define CSR_MCYCLE 0xb00
#define CSR_MINSTRET 0xb02

// csr R/W borrowed from OpenSBI project under BSD 2-clause license
#define __ASM_STR(x)	#x
#define csr_read(csr)                                           \
	({                                                      \
		register unsigned long __v;                     \
		__asm__ __volatile__("csrr %0, " __ASM_STR(csr) \
				     : "=r"(__v)                \
				     :                          \
				     : "memory");               \
		__v;                                            \
	})

#define csr_write(csr, val)                                        \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrw " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define csr_set(csr, val)                                          \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrs " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define csr_clear(csr, val)                                        \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrc " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})



#endif // __CSR_H__ 