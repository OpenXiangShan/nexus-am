#ifndef __ADDR_S__
#define __ADDR_S__
typedef union {
	lnaddr_t lnaddr;
	struct {
		uint32_t OFFSET : 12;
		uint32_t PAGE : 10;
		uint32_t DIR : 10;
	};
} ln_addr;

typedef union {
	SegDesc sr_des;
	uint32_t val[2];
} sreg_desc;

typedef union {
	GateDesc gate;
	uint32_t val[2];
} gate_desc;

typedef union {
	uint32_t val;
	struct {
		uint32_t base15 : 16;
		uint32_t base23 : 8;
		uint32_t base31 : 8;
	};
} base_addr;

typedef union {
	uint32_t val : 20;
	struct {
		uint32_t limit15 : 16;
		uint32_t limit19 : 4;
	};
} limit;

typedef union {
	uint32_t val;
	struct {
		uint16_t offset15;
		uint16_t offset31;
	};
} offset;

#endif
