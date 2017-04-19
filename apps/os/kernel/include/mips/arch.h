#ifndef __ARCH_H__
#define __ARCH_H__

#define KERNEL_MODE 0
#define USER_MODE 3
#define NULL ((void*)0)

typedef struct CP0 {
	unsigned int index,entrylo0,entrylo1,
		     context,pagemask,badvaddr,
		     count,entryhi,compare,status,
		     cause,epc,prid,watchlo,watchhi,
		     taglo,taghi;
} CP0;

typedef struct _RegSet {
	unsigned int zero,
		     at,
		     v0,v1,
		     a0,a1,a2,a3,
		     t0,t1,t2,t3,t4,t5,t6,t7,
		     s0,s1,s2,s3,s4,s5,s6,s7,
		     k0,k1,
		     t8,t9,
		     gp,sp,fp,ra;
} _RegSet;

#endif
