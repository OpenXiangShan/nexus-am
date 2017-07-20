#ifndef __ARCH_H__
#define __ARCH_H__

struct _RegSet {
	uint32_t at,
	v0,v1,
	a0,a1,a2,a3,
	t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
	s0,s1,s2,s3,s4,s5,s6,s7,
	k0,k1,
	gp,sp,fp,ra,
	epc, cause, status, badvaddr;
};

#define REG1(regs) ((regs)->a0)
#define REG2(regs) ((regs)->a1)
#define REG3(regs) ((regs)->a2)
#define REG4(regs) ((regs)->a3)

#ifdef __cplusplus
extern "C" {
#endif

void _ioe_init();
uintptr_t _uptime();
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h);
void _draw_sync();

#ifdef __cplusplus
}


#endif
#endif
