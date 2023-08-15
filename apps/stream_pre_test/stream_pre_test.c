#include <klib.h>
#include "stream_pre_test.h"

int main(){
    unsigned long long cycles,instrs;
    stream_pre(&instrs,&cycles);
    printf("the test of stream_pre \n");
    printf("instrs %d cycles %d\n",instrs,cycles);
}