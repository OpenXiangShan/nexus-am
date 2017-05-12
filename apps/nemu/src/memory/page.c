#include "common.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"
#include "x86-inc/addr_struct.h"
#include "tlb.h"
#include "time.h"
#include <stdlib.h>

uint32_t hwaddr_read(hwaddr_t addr, size_t len);

hwaddr_t page_translate(lnaddr_t addr) {
	if(cpu.cr0.paging == 0 || cpu.cr0.protect_enable == 0)
		/*DON'T TRANSLATE*/
		return addr;

	/*ASSERT CR3*/
	if(cpu.pre_cr3.val != cpu.cr3.val)
	{
		int j = 0;
		for(j = 0;j < TLB_LEN; j++)
			tlb[j].valid = false;
		cpu.pre_cr3.val = cpu.cr3.val;
	}

	/*SEARCH TLB*/
	hwaddr_t ret_addr = 0;
	bool sear_suc = search_tlb(addr, (hwaddr_t*)&ret_addr);
	if(sear_suc == true)	
		/*hit tlb[i]*/
		return ret_addr;

	/*SEARCH PDE*/
	ln_addr l_add;
	l_add.lnaddr = addr;
	PDE page_dir_entry;
	page_dir_entry.val = hwaddr_read((cpu.cr3.page_directory_base << 12) + l_add.DIR * 4, 4);

	/* should not reach here */
	if(page_dir_entry.present == 0)
	{
		Log("addr = %x",addr);
		assert(0);
	}	
	
	PTE page_table_entry;
	page_table_entry.val = hwaddr_read((page_dir_entry.page_frame << 12) + l_add.PAGE * 4, 4);
	/* should not reach here */
	if(page_table_entry.present == 0)
	{
		Log("addr = %x",addr);
		assert(0);
	}

	ret_addr = (page_table_entry.page_frame << 12) + l_add.OFFSET;

	/*SET TLB*/
	int i = 0;
	for(i = 0;i < TLB_LEN; i++)
	{
		if(tlb[i].valid == false)
		{
			tlb[i].pte.page_frame = page_table_entry.page_frame;
			tlb[i].tag = addr >> 12;
			tlb[i].valid = true;
			return ret_addr;
		}
	}	
	srand(time(0));
	i = rand()%TLB_LEN;
	tlb[i].pte.page_frame = page_table_entry.page_frame;
	tlb[i].tag = addr >> 12;
	tlb[i].valid = true;
	return ret_addr;
}
