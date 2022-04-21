#ifndef __PMP_H__
#define __PMP_H__

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
#define CSR_PMPCFG0			0x3a0
#define CSR_PMPADDR0	    0x3b0

unsigned long csr_read_num(int csr_num)
{
#define switchcase_csr_read(__csr_num, __val)		\
	case __csr_num:					\
		__val = csr_read(__csr_num);		\
		break;
#define switchcase_csr_read_2(__csr_num, __val)	\
	switchcase_csr_read(__csr_num + 0, __val)	\
	switchcase_csr_read(__csr_num + 1, __val)
#define switchcase_csr_read_4(__csr_num, __val)	\
	switchcase_csr_read_2(__csr_num + 0, __val)	\
	switchcase_csr_read_2(__csr_num + 2, __val)
#define switchcase_csr_read_8(__csr_num, __val)	\
	switchcase_csr_read_4(__csr_num + 0, __val)	\
	switchcase_csr_read_4(__csr_num + 4, __val)
#define switchcase_csr_read_16(__csr_num, __val)	\
	switchcase_csr_read_8(__csr_num + 0, __val)	\
	switchcase_csr_read_8(__csr_num + 8, __val)
#define switchcase_csr_read_32(__csr_num, __val)	\
	switchcase_csr_read_16(__csr_num + 0, __val)	\
	switchcase_csr_read_16(__csr_num + 16, __val)
#define switchcase_csr_read_64(__csr_num, __val)	\
	switchcase_csr_read_32(__csr_num + 0, __val)	\
	switchcase_csr_read_32(__csr_num + 32, __val)

	unsigned long ret = 0;

	switch (csr_num) {
	switchcase_csr_read_16(CSR_PMPCFG0, ret)
	switchcase_csr_read_64(CSR_PMPADDR0, ret)
	};

	return ret;

#undef switchcase_csr_read_64
#undef switchcase_csr_read_32
#undef switchcase_csr_read_16
#undef switchcase_csr_read_8
#undef switchcase_csr_read_4
#undef switchcase_csr_read_2
#undef switchcase_csr_read
}

void csr_write_num(int csr_num, unsigned long val)
{
#define switchcase_csr_write(__csr_num, __val)		\
	case __csr_num:					\
		csr_write(__csr_num, __val);		\
		break;
#define switchcase_csr_write_2(__csr_num, __val)	\
	switchcase_csr_write(__csr_num + 0, __val)	\
	switchcase_csr_write(__csr_num + 1, __val)
#define switchcase_csr_write_4(__csr_num, __val)	\
	switchcase_csr_write_2(__csr_num + 0, __val)	\
	switchcase_csr_write_2(__csr_num + 2, __val)
#define switchcase_csr_write_8(__csr_num, __val)	\
	switchcase_csr_write_4(__csr_num + 0, __val)	\
	switchcase_csr_write_4(__csr_num + 4, __val)
#define switchcase_csr_write_16(__csr_num, __val)	\
	switchcase_csr_write_8(__csr_num + 0, __val)	\
	switchcase_csr_write_8(__csr_num + 8, __val)
#define switchcase_csr_write_32(__csr_num, __val)	\
	switchcase_csr_write_16(__csr_num + 0, __val)	\
	switchcase_csr_write_16(__csr_num + 16, __val)
#define switchcase_csr_write_64(__csr_num, __val)	\
	switchcase_csr_write_32(__csr_num + 0, __val)	\
	switchcase_csr_write_32(__csr_num + 32, __val)

	switch (csr_num) {
	switchcase_csr_write_16(CSR_PMPCFG0, val)
	switchcase_csr_write_64(CSR_PMPADDR0, val)
	};

#undef switchcase_csr_write_64
#undef switchcase_csr_write_32
#undef switchcase_csr_write_16
#undef switchcase_csr_write_8
#undef switchcase_csr_write_4
#undef switchcase_csr_write_2
#undef switchcase_csr_write
}

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

void csr_set_num(int csr_num, unsigned long val)
{
#define switchcase_csr_set(__csr_num, __val)		\
	case __csr_num:					\
		csr_set(__csr_num, __val);		\
		break;
#define switchcase_csr_set_2(__csr_num, __val)	\
	switchcase_csr_set(__csr_num + 0, __val)	\
	switchcase_csr_set(__csr_num + 1, __val)
#define switchcase_csr_set_4(__csr_num, __val)	\
	switchcase_csr_set_2(__csr_num + 0, __val)	\
	switchcase_csr_set_2(__csr_num + 2, __val)
#define switchcase_csr_set_8(__csr_num, __val)	\
	switchcase_csr_set_4(__csr_num + 0, __val)	\
	switchcase_csr_set_4(__csr_num + 4, __val)
#define switchcase_csr_set_16(__csr_num, __val)	\
	switchcase_csr_set_8(__csr_num + 0, __val)	\
	switchcase_csr_set_8(__csr_num + 8, __val)
#define switchcase_csr_set_32(__csr_num, __val)	\
	switchcase_csr_set_16(__csr_num + 0, __val)	\
	switchcase_csr_set_16(__csr_num + 16, __val)
#define switchcase_csr_set_64(__csr_num, __val)	\
	switchcase_csr_set_32(__csr_num + 0, __val)	\
	switchcase_csr_set_32(__csr_num + 32, __val)

	switch (csr_num) {
	switchcase_csr_set_16(CSR_PMPCFG0, val)
	switchcase_csr_set_64(CSR_PMPADDR0, val)
	};

#undef switchcase_csr_set_64
#undef switchcase_csr_set_32
#undef switchcase_csr_set_16
#undef switchcase_csr_set_8
#undef switchcase_csr_set_4
#undef switchcase_csr_set_2
#undef switchcase_csr_set
}

void csr_clear_num(int csr_num, unsigned long val)
{
#define switchcase_csr_clear(__csr_num, __val)		\
	case __csr_num:					\
		csr_clear(__csr_num, __val);		\
		break;
#define switchcase_csr_clear_2(__csr_num, __val)	\
	switchcase_csr_clear(__csr_num + 0, __val)	\
	switchcase_csr_clear(__csr_num + 1, __val)
#define switchcase_csr_clear_4(__csr_num, __val)	\
	switchcase_csr_clear_2(__csr_num + 0, __val)	\
	switchcase_csr_clear_2(__csr_num + 2, __val)
#define switchcase_csr_clear_8(__csr_num, __val)	\
	switchcase_csr_clear_4(__csr_num + 0, __val)	\
	switchcase_csr_clear_4(__csr_num + 4, __val)
#define switchcase_csr_clear_16(__csr_num, __val)	\
	switchcase_csr_clear_8(__csr_num + 0, __val)	\
	switchcase_csr_clear_8(__csr_num + 8, __val)
#define switchcase_csr_clear_32(__csr_num, __val)	\
	switchcase_csr_clear_16(__csr_num + 0, __val)	\
	switchcase_csr_clear_16(__csr_num + 16, __val)
#define switchcase_csr_clear_64(__csr_num, __val)	\
	switchcase_csr_clear_32(__csr_num + 0, __val)	\
	switchcase_csr_clear_32(__csr_num + 32, __val)

	switch (csr_num) {
	switchcase_csr_clear_16(CSR_PMPCFG0, val)
	switchcase_csr_clear_64(CSR_PMPADDR0, val)
	};

#undef switchcase_csr_clear_64
#undef switchcase_csr_clear_32
#undef switchcase_csr_clear_16
#undef switchcase_csr_clear_8
#undef switchcase_csr_clear_4
#undef switchcase_csr_clear_2
#undef switchcase_csr_clear
}

#endif // __PMP_H__ 