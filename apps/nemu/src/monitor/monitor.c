#include "nemu.h"

#define ENTRY_START 0x100000

extern uint8_t entry [];
extern uint32_t entry_len;
extern char *exec_file;

void load_elf_tables(int, char *[]);
void init_regex();
void init_wp_list();
void init_ddr3();
void init_cache();
void init_device();
void init_sdl();
static void init_CR0();
void init_tlb();

FILE *log_fp = NULL;

static void init_sreg() {
	cpu.ES.base = 0;
	cpu.ES.limit = 0xfffff;
	cpu.ES.valid = false;
	cpu.DS.base = 0;
	cpu.DS.limit = 0xfffff;
	cpu.DS.valid = false;
	cpu.SS.base = 0;
	cpu.SS.limit = 0xfffff;
	cpu.SS.valid = false;
	cpu.CS.base = 0;
	cpu.CS.limit = 0xfffff;
	cpu.CS.valid = false;
};

static void init_CR0() {
	cpu.cr0.protect_enable = 0;
	cpu.cr0.paging = 0;
}

static void init_log() {
	log_fp = fopen("log.txt", "w");
	Assert(log_fp, "Can not open 'log.txt'");
}

static void welcome() {
	printf("Welcome to NEMU!\nThe executable is %s.\nFor help, type \"help\"\n",
			exec_file);
}

void init_monitor(int argc, char *argv[]) {
	/* Perform some global initialization */

	/* Open the log file. */
	init_log();

	/* Load the string table and symbol table from the ELF file for future use. */
	load_elf_tables(argc, argv);

	/* Compile the regular expressions. */
	init_regex();

	/* Initialize the watchpoint link list. */
	init_wp_list();
	
	init_device();

	init_sdl();

	/* Display welcome message. */
	welcome();
}

#ifdef USE_RAMDISK
static void init_ramdisk() {
	int ret;
	const int ramdisk_max_size = 0xa0000;
	FILE *fp = fopen(exec_file, "rb");
	Assert(fp, "Can not open '%s'", exec_file);

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	Assert(file_size < ramdisk_max_size, "file size(%zd) too large", file_size);

	fseek(fp, 0, SEEK_SET);
	ret = fread(hwa_to_va(0), file_size, 1, fp);
	assert(ret == 1);
	fclose(fp);
}
#endif

static void load_entry() {
	int ret;
	FILE *fp = fopen("entry", "rb");
	Assert(fp, "Can not open 'entry'");

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	ret = fread(hwa_to_va(ENTRY_START), file_size, 1, fp);
	assert(ret == 1);
	fclose(fp);
}

void restart() {
	/* Perform some initialization to restart a program */
#ifdef USE_RAMDISK
	/* Read the file with name `argv[1]' into ramdisk. */
	init_ramdisk();
#endif

	/* Read the entry code into memory. */
	
	load_entry();

	/* Set the initial instruction pointer. */
	cpu.eip = ENTRY_START;

	/* Initialize DRAM. */
	init_ddr3();

	/* Initialize EFLAGS. */
	cpu.EFLAGS.A = 0x0;
	cpu.EFLAGS.OF = 0x0;
	cpu.EFLAGS.DF = 0x0;
	cpu.EFLAGS.IF = 0x0;
	cpu.EFLAGS.B = 0x0;
	cpu.EFLAGS.SF = 0x0;
	cpu.EFLAGS.ZF = 0x0;
	cpu.EFLAGS.C = 0x0;
	cpu.EFLAGS.PF = 0x0;
	cpu.EFLAGS.D = 0x1;
	cpu.EFLAGS.CF = 0x0;

	cpu.INTR = false;

	/* Initialize CACHE. */
	init_cache();

	/* Initialize CR0. */
	init_CR0();
	
	/* Initialize segment registers */
	init_sreg();

	/* Initialize tlb */
	init_tlb();
	
	/* Initialize CR3 */
	cpu.pre_cr3.val = cpu.cr3.val;
}
