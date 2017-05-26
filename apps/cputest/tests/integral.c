#include "trap.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
	int carry=(a & 0xffff) >= 0x8000;
	return (a>>16) + carry;
}

static inline FLOAT int2F(int a) {
	return a<<16;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
	return a*b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
	return a/b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT sqrt(FLOAT);
FLOAT pow(FLOAT, FLOAT);


FLOAT f(FLOAT x) { 
	/* f(x) = 1/(1+25x^2) */
	return F_div_F(int2F(1), int2F(1) + F_mul_int(F_mul_F(x, x), 25));
}

FLOAT computeT(int n, FLOAT a, FLOAT b, FLOAT (*fun)(FLOAT)) {
	int k;
	FLOAT s,h;
	h = F_div_int((b - a), n);
	s = F_div_int(fun(a) + fun(b), 2 );
	for(k = 1; k < n; k ++) {
		s += fun(a + F_mul_int(h, k));
	}
	s = F_mul_F(s, h);
	return s;
}

int main() { 
	FLOAT a = computeT(10, f2F(-1.0), f2F(1.0), f);
	FLOAT ans = f2F(0.551222);

	nemu_assert(Fabs(a - ans) < f2F(1e-4));
	HIT_GOOD_TRAP;
	return 0;
}

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	//return ((a>>4)*(b>>4))>>8;
	//long long r=((long long)a * b)>>16;
	//return (FLOAT)(r & 0xffffffff);	//if I use this way, shrd instruction is not surported.

	union {
		long long l;
		struct {
			unsigned short twobyte1,twobyte2,twobyte3,twobyte4;
		};
	}temp;

	temp.l=(long long)a*b;
	return (unsigned int)(temp.twobyte2) + (((unsigned int)temp.twobyte3)<<16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	return (a<<13)/(b>>3);
	//This is other pepele's help.
/*	unsigned sign=(a & 0x80000000) ^ (b & 0x80000000);
	unsigned ua=a & 0x7fffffff,
			 ub=b & 0x7fffffff;
	int result=((ua/ub)<<16);
	unsigned remain=ua%ub;
	int i=0;
	for(;i<16;i++)
	{
		remain<<=1;
		if(sign==0)
			result+=(remain/ub)<<(15-i);
		else
			result-=(remain/ub)<<(15-i);
		remain%=ub;
	}
	return result | sign;
*/	
}

FLOAT f2F(float a) {
	union fi {
		float f;
		int i;
	}temp;
	temp.f=a;

	int exp=((temp.i & 0x7f800000)>>23) - 127;
	int m=(temp.i & 0x7fffff) + 0x800000;
	int sign=0x80000000 & temp.i;
	//Assert(exp<=14, "float to FLOAT overflow!");

	FLOAT result;
	if(exp>7)
		result=m<<(exp-7);	
	else
		result=m>>(7-exp);

	if(sign==0)
		return result;
	else
		return -result;
}

FLOAT Fabs(FLOAT a) {
	return (a>=0) ? a : (-a);
}


FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

