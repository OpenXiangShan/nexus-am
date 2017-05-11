#ifndef __CPU_H__
#define __CPU_H__

#include "common.h"

typedef struct {
  byte CF;
  byte PF;
  byte AF;
  byte ZF;
  byte SF;
  byte TF;
  byte IF;
  byte DF;
  byte OF;
  byte IO_PL;
  byte NT;
  byte RF;
  byte VM;
} EFLAGS;

typedef union {
  word val;
  struct {
    byte RH;
    byte RL;
  };
} RX;

typedef union {
  dword val;
  struct {
    RX rx;
    RX erx;
  };
} ERX;

typedef struct {
  byte PE;
  byte MP;
  byte EM;
  byte TS;
  byte ET;
  byte PG;
} CR0;

typedef struct {
  byte RPL;
  byte TI;
  word INDEX;
} SEL;

typedef struct {
  word LIMIT;
  dword BASE;
} DTR;

typedef struct {
  ERX eax,ebx,ecx,edx,edi,esi,esp,ebp;
  dword eip;
  EFLAGS eflags;
  CR0 cr0;
  SEL cs,es,ds,ss;
  DTR gdtr,idtr;
} CPU;

typedef struct {
  dword op;
  dword dest, src1, src2;
} instruction;

#endif
