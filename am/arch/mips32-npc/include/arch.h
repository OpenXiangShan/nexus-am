#ifndef __ARCH_H__
#define __ARCH_H__

struct _RegSet {
	uint32_t at,
	v0,v1,
	a0,a1,a2,a3,
	t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
	s0,s1,s2,s3,s4,s5,s6,s7,
	k0,k1,
	gp,sp,fp,ra;
};

#endif
