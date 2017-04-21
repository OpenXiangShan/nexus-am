#ifndef __ARCH_H__
#define __ARCH_H__

typedef unsigned int u32;
typedef int i32;
typedef unsigned short u16;
typedef short int i16;
typedef unsigned char u8;
typedef char i8;

typedef struct _RegSet {
	u32 at,
	v0,v1,
	a0,a1,a2,a3,
	t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
	s0,s1,s2,s3,s4,s5,s6,s7,
	k0,k1,
	gp,sp,fp,ra;
} _RegSet;

#endif
