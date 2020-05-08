#include "trap.h"

/*
    flw, fsw, fcvt, fadd, fmul, fdiv will be used in this program.
*/

#define N 10
double a[N];
float b[N];
int c[N];

int main() {
  int i, j;
  for(i = 0; i < N; i ++)
    a[i] = i;
  for(i = 0; i < N; i ++)
    for(j = 1; j < N + 1; j ++)
      a[i] *= j;
  for(i = 0; i < N; i ++)
    for(j = 1; j < N + 1; j ++)
      a[i] /= j;

  for(i = 0; i < N; i ++){
    b[i] = a[i];
    if(i % 2 == 0){
      c[i] = a[i];
    }else{
      c[i] = b[i];
    }
    nemu_assert(a[i] == i);
    nemu_assert(b[i] == i);
    nemu_assert(c[i] == i);
  }

  return 0;
}


