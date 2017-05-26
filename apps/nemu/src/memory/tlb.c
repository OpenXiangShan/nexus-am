#include "common.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"
#include "tlb.h"

bool search_tlb(lnaddr_t addr, hwaddr_t *h_addr) {
	uint32_t virtual_page = addr >> 12;
	uint32_t offset = addr & TLB_MASK;
	int i;
	for(i = 0;i < TLB_LEN;i++)
	{
		if(tlb[i].tag == virtual_page && tlb[i].valid == true)
		{
			*h_addr = (tlb[i].pte.page_frame << 12) + offset;
			return true;
		}
	}
	return false;
}

void init_tlb() {
	int i;
	for(i = 0; i < TLB_LEN; i++)
		tlb[i].valid = false;
}
