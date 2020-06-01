
#include "trap.h"

/*
    use recursive function to test fld/fsd
*/

#define N 10

double fibDouble(double x){
    if(x < 2.0){
        return x;
    }
    else{
        return fibDouble(x-1) + fibDouble(x-2);
    }
}


int fibInt(int x){
    if(x <= 1){
        return x;
    }
    else{
        return fibInt(x-1) + fibInt(x-2);
    }
}

int main() {
  
  nemu_assert(fibInt(N)==fibDouble(N));
  
  return 0;
}


