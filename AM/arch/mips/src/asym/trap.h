#ifndef trap_h
#define trap_h

typedef struct {
	int Epc;
	int Cause;
	int Status;
	int BadVaddr;
} CP0;

typedef struct {
	int regs0_25[26];
	CP0 cp0;
	int regs28_32[4];
} trap_frame;
#endif
