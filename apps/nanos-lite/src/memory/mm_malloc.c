#include "common.h"
#include "memory.h"
#include "x86.h"

static PTE uptable[PHY_MEM / PT_SIZE][NR_PTE] align_to_page;

/* allocating page tables starting from the very begining */
static int pt_idx = 0;
/* allocating physical pages starting from the 4096th one (4096 * 4KB = 16MB) */
static int pf_idx = 4096;	

PDE* get_updir();
uint32_t get_ucr3();

static inline uint32_t va2pa(uint32_t cr3, uint32_t va) {
	uint32_t pde = ((uint32_t *)((uint32_t)pa_to_va(cr3) & ~0xfff))[va >> 22];
	assert(pde & 1);
	uint32_t pte = ((uint32_t *)((uint32_t)pa_to_va(pde) & ~0xfff))[(va >> 12) & 0x3ff];
	assert(pte & 1);
	uint32_t pa = (pte & ~0xfff) | (va & 0xfff);
	return pa;
}

uint32_t mm_malloc(uint32_t va, int len) {
	if(len <= 0) return 0;

	PDE *updir = get_updir();
	uint32_t ucr3 = get_ucr3();

	uint32_t va_temp = va;
	uint32_t va_end = va + len - 1;
	len = ((va + len - 1) & ~PAGE_MASK) + PAGE_SIZE - va;
//	Log("va_end = %x, len2 = %d", va_end, len);
	for(; len > 0; len -= PAGE_SIZE, va += PAGE_SIZE) {
		int pde_idx = va / PT_SIZE;
		PTE *pt;

		if(updir[pde_idx].present == 0) {
			pt = uptable[pt_idx ++];
			updir[pde_idx].val = make_pde(va_to_pa(pt));
			memset(pt, 0, NR_PTE * sizeof(NR_PTE));
		}
		else {
			pt = (void *)pa_to_va(updir[pde_idx].val & ~0xfff);
		}
		
		int pte_idx = (va % PT_SIZE) / PAGE_SIZE;
		if(pt[pte_idx].present == 0) {
			assert(pf_idx < 32 * 1024);	// not full

			pt[pte_idx].val = make_pte(pf_idx * PAGE_SIZE);
			pf_idx ++;
		}
	}

	va2pa(ucr3, va_end);
	return va2pa(ucr3, va_temp);
}
