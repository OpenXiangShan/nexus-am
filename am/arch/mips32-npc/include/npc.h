#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((volatile void *)0x50000000)
#define SCR_WIDTH 400
#define SCR_HEIGHT 300
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)
#define SERIAL_PORT ((volatile char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define GPIO_TRAP ((volatile char *)0x40000000)
#define HZ 50000000
#define MAX_MEMORY_SIZE 0x4000000
#define INTERVAL 300000
#define REAL_TIMER_BASE ((volatile char *)0x41c00000)
#define INT_TIMER_BASE ((volatile char *)0x41c10000)
#define PERF_COUNTER_BASE ((volatile char *)0xc0000000)

typedef struct {
	uint32_t IE   : 1;
	uint32_t EXL  : 1;
	uint32_t ERL  : 1;
	uint32_t R0   : 1;

	uint32_t UM   : 1;
	uint32_t UX   : 1;
	uint32_t SX   : 1;
	uint32_t KX   : 1;

	uint32_t IM   : 8;

	uint32_t Impl : 2;
	uint32_t _0   : 1;
	uint32_t NMI  : 1;
	uint32_t SR   : 1;
	uint32_t TS   : 1;

	uint32_t BEV  : 1;
	uint32_t PX   : 1;

	uint32_t MX   : 1;
	uint32_t RE   : 1;
	uint32_t FR   : 1;
	uint32_t RP   : 1;
	uint32_t CU   : 4;
} cp0_status_t;

typedef struct {
	uint32_t _5 : 2;
	uint32_t ExcCode : 5;
	uint32_t _4 : 1;
	uint32_t IP : 8;

	uint32_t _3 : 6;
	uint32_t WP : 1;
	uint32_t IV : 1;

	uint32_t _2 : 4;
	uint32_t CE : 2;
	uint32_t _1 : 1;
	uint32_t BD : 1;
} cp0_cause_t;

#define CP0_BADVADDR 8
#define CP0_COUNT    9
#define CP0_COMPARE  11
#define CP0_STATUS   12
#define CP0_CAUSE    13
#define CP0_EPC      14

#define IP_TIMER_MASK 0x80

#define EXC_INTR    0
#define EXC_TLB     1
#define EXC_TLBL    2
#define EXC_TLBS    3
#define EXC_AdEL    4
#define EXC_AdES    5
#define EXC_IBE     6
#define EXC_DBE     7
#define EXC_SYSCALL 8
#define EXC_BP      9
#define EXC_RI      10
#define EXC_CPU     11
#define EXC_OV      12
#define EXC_TRAP    13

#define MFC0(dst, src, sel) \
asm volatile("mfc0 %0, $"_STR(src)", %1\n\t":"=r"(dst):"i"(sel))

#define MTC0(dst, src, sel) \
asm volatile("mtc0 %0, $"_STR(dst)", %1\n\t"::"g"(src),"i"(sel))

#define _STR(x) _VAL(x)
#define _VAL(x) #x


char in_byte();
void out_byte(char);

#endif
