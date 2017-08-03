//==========================================================
// MIPS CPU binary executable file loader
//
// Main Function:
// 1. Loads binary excutable file into distributed memory
// 2. Waits MIPS CPU for finishing program execution
//
// Author:
// Yisong Chang (changyisong@ict.ac.cn)
//
// Revision History:
// 14/06/2016	v0.0.1	Add cycle counte support
//==========================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <unistd.h>  
#include <sys/mman.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include <elf.h>

#include <assert.h>
#include <sys/time.h>
#include <time.h>

#define BRAM_TOTAL_SIZE		(1 << 13)
#define BRAM_BASE_ADDR		0x40002000

#define DDR_TOTAL_SIZE		(1 << 28)
#define DDR_BASE_ADDR		0x10000000

#define GPIO_RESET_TOTAL_SIZE	(1 << 16)
#define GPIO_RESET_BASE_ADDR	0x41200000

#define GPIO_TRAP_TOTAL_SIZE	(1 << 12)
#define GPIO_TRAP_BASE_ADDR	0x40000000
#define GPIO_TRAP_INIT		0xff

void *bram_base;
void *ddr_base;
volatile uint32_t *gpio_reset_base;
volatile uint32_t *gpio_trap_base;
int	fd;

void* mips_addr(uintptr_t p) {
	if(p < BRAM_TOTAL_SIZE) {
		return bram_base + p;
	}
	else if(p >= DDR_BASE_ADDR && p < DDR_BASE_ADDR + DDR_TOTAL_SIZE) {
		return ddr_base + (p - DDR_BASE_ADDR);
	}
	else {
		printf("Illegal address 0x%08x\n", p);
		exit(1);
	}
}

void loader(char *file) {
	FILE *fp = fopen(file, "rb");
	assert(fp);
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph = NULL;
	int i;
	uint8_t buf[4096];

	// the program header should be located within the first
	// 4096 byte of the ELF file
	fread(buf, 4096, 1, fp);
	elf = (void *)buf;

	// TODO: fix the magic number with the correct one
	const uint32_t elf_magic = 0x464c457f; // 0xBadC0de;
	uint32_t *p_magic = (void *)buf;
	// check the magic number
	assert(*p_magic == elf_magic);
	 assert(elf->e_entry == 0x10000000);

	for(i = 0, ph = (void *)buf + elf->e_phoff; i < elf->e_phnum; i ++) {
		// scan the program header table, load each segment into memory
		if(ph[i].p_type == PT_LOAD) {
			uint32_t va = ph[i].p_vaddr;

			// TODO: read the content of the segment from the ELF file
			// to the memory region [VirtAddr, VirtAddr + FileSiz)
			fseek(fp, ph[i].p_offset, SEEK_SET);
			fread(mips_addr(va), ph[i].p_filesz, 1, fp);

			// TODO: zero the memory region
			// [VirtAddr + FileSiz, VirtAddr + MemSiz)
			memset(mips_addr(va + ph[i].p_filesz), 0, ph[i].p_memsz - ph[i].p_filesz);
		}
	}

	fclose(fp);
}

void* create_map(size_t size, int fd, off_t offset) {
	void *base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
	
	if (base == NULL) {
		perror("init_mem mmap failed:");
		close(fd);
		exit(1);
	}

	return base;
}

void init_map() {
	fd = open("/dev/mem", O_RDWR|O_SYNC);  
	if (fd == -1)  {  
		perror("init_map open failed:");
		exit(1);
	} 

	bram_base = create_map(BRAM_TOTAL_SIZE, fd, BRAM_BASE_ADDR);
	gpio_reset_base = create_map(GPIO_RESET_TOTAL_SIZE, fd, GPIO_RESET_BASE_ADDR);
	gpio_trap_base = create_map(GPIO_TRAP_TOTAL_SIZE, fd, GPIO_TRAP_BASE_ADDR);
	ddr_base = create_map(DDR_TOTAL_SIZE, fd, DDR_BASE_ADDR);
}

void resetn(int val) {
	gpio_reset_base[0] = val;
}

uint8_t read_trap() {
	return gpio_trap_base[2];
}

void write_trap(uint8_t val) {
	gpio_trap_base[0] = val;
}

uint8_t wait_for_finish() {
	uint8_t ret;
	while((ret = read_trap()) == GPIO_TRAP_INIT);
	return ret;
}

void finish_map() {
	munmap((void *)bram_base, BRAM_TOTAL_SIZE);
	munmap((void *)gpio_reset_base, GPIO_RESET_TOTAL_SIZE);
	munmap((void *)gpio_trap_base, GPIO_TRAP_TOTAL_SIZE);
	munmap((void *)ddr_base, DDR_TOTAL_SIZE);
	close(fd);
}

int main(int argc, char *argv[]) {
    time_t start, end;	
	/* map some devices into the address space of this program */
	init_map();

	/* reset MISP CPU */
	resetn(0);

	/* load MIPS binary executable file to distributed memory */
	loader(argv[1]);

	write_trap(GPIO_TRAP_INIT);

	/* finish reset MIPS CPU */
	resetn(1);

	/* start timing */
  start = time(NULL);

	/* wait for MIPS CPU finish  */
	printf("Waiting MIPS CPU to finish...\n");
	uint8_t ret = wait_for_finish();

	/* finish timing */
	end = time(NULL);
	printf("Total time: %.2f s\n", difftime(end, start));
	printf("MIPS CPU Execution is finished...\n");

	printf("HIT %s TRAP!\n", (ret == 0 ? "GOOD" : "BAD"));

	/* reset MISP CPU */
	resetn(0);

	finish_map();

	return 0; 
} 
