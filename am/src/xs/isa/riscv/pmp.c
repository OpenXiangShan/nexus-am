#include <xs.h>
#include <csr.h>
#include <klib.h>
/*
 * Note that PMP should only be set on machine mode
 * pmp test in amtest call these functions, thus they may serve as a functional verification
 */

/* 
 * Hence, when the PMP settings are modified, M-mode software must synchronize the 
 * PMP settings with the virtual memory system and any PMP or address-translation
 * caches. This is accomplished by executing an SFENCE.VMA instruction with rs1=x0 
 * and rs2=x0, after the PMP CSRs are written.
 */

// Note that csr utility functions are derived from OpenSBI under BSD 2-clause license

/*
 * CSR read function
 * Note that inline assembly does not allow dynamic csr address
 * we implement by defining cases for all possible addresses
 * csr_num: the address of csr to be read
 * return value: given csr value
 */
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

/*
 * CSR write function
 * csr_num: the address of csr, val: the value to be written
 */
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

/*
 * CSR set function
 * csr_num: the address of csr, val: value that will be set, 0 bits are ignored
 */
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

/*
 * CSR clear function
 * csr_num: the address of csr to be cleared, val: the value to be cleared, 0 bits are ignored
 */
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

/*
 * PMP initialize function
 * must be called under MACHINE mode
 * set all pmpaddr registers to 0xffff_ffff_ffff_ffff
 * set pmpcfg8 to NAPOT mode such that by default we allow all memory access from S mode
 */

void init_pmp() {
  // set all addr registers to disable possible access fault
  for (int i = 0; i < PMP_COUNT; i++) {
    csr_write_num(PMPADDR_BASE + i, -1L);
  }
  // set PMP to access all memory in S-mode
  // asm volatile("csrw pmpaddr8, %0" : : "r"(-1));
  // the last pmp pair is used to enable all access (in current case is pmp15)
  asm volatile("csrw pmpcfg2, %0" : : "r"((long)31<<(8*7))); 

  asm volatile("sfence.vma");
}
/*
 * PMP enable function (NAPOT)
 * pmp_reg: PMP register pairs to be used, ranging from 0 to 15 for XS
 * pmp_addr: starting position to be protected
 * pmp_size: total size to be protected, must be power of 2
 * lock: once set, given PMP registers cannot be reconfigured until core reset
 * permission: R/W/X
 * must be called under MACHINE mode
 */
void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission) {
  // by default using NAPOT
  assert((pmp_size & (pmp_size - 1)) == 0); // must be power of 2
  assert(pmp_reg < PMP_COUNT);
  if (pmp_size != 4) {
    // adjust pmp addr according to pmp size
    uintptr_t append = (pmp_size >> 1) - 1;
    pmp_addr |= append;
  }
  csr_write_num(PMPADDR_BASE + pmp_reg, pmp_addr >> 2);

  uintptr_t set_content = permission | (lock << 7) | (pmp_size == 4 ? 2 : 3) << 3;
  uintptr_t cfg_offset = pmp_reg > 7 ? 2 : 0;
  uintptr_t cfg_shift = pmp_reg & 0x7;
  // printf("addr %llx, cfg offset %d, shift %d, set_content %d\n", pmp_addr, cfg_offset, cfg_shift, set_content);
  csr_set_num(PMPCFG_BASE + cfg_offset, set_content << (cfg_shift * 8));
  asm volatile("sfence.vma");
}
/*
 * PMP enable function (TOR)
 * use two consecutive pmpaddr registers to hold the range
 * pmp_reg: lower PMP register, pmp_reg + 1 will also be set
 * pmp_addr: starting position to be protected
 * pmp_size: total size to be protected
 * lock: once set, given PMP registers cannot be reconfigured until core reset
 * permission: R/W/X
 * must be called under MACHINE mode
 */
void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission) {
    // similar interface but different implementation
    // pmp reg and pmp reg + 1 will be used
    assert(pmp_reg < PMP_COUNT);
    csr_write_num(PMPADDR_BASE + pmp_reg, (pmp_addr + pmp_size) >> 2);
    if (pmp_reg) {
      csr_write_num(PMPADDR_BASE + pmp_reg - 1, pmp_addr >> 2);
    }
    // printf("finished writing ADDR\n");
    uintptr_t set_content = permission | (lock << 7) | (1 << 3);
    uintptr_t cfg_offset = pmp_reg > 7 ? 2 : 0;
    uintptr_t cfg_shift = pmp_reg & 0x7;
    // printf("addr %llx, cfg offset %d, shift %d, set_content %d\n", pmp_addr, cfg_offset, cfg_shift, set_content);
    csr_set_num(PMPCFG_BASE + cfg_offset, set_content << (cfg_shift * 8));
    // printf("finished writing CFG\n");
    asm volatile("sfence.vma");
}

/*
 * PMP disable function
 * disable by setting corresponding pmpaddr to 0xffff_ffff_ffff_ffff
 * must be called under MACHINE mode
 */
void disable_pmp(uintptr_t pmp_reg) {
  // just set pmp addr to max
  csr_write_num(PMPADDR_BASE + pmp_reg, -1L);
  asm volatile("sfence.vma");
}