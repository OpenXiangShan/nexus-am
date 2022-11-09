#include <klib.h>

__attribute__((aligned(256)))
void busy_loop(unsigned long long* instr_count, unsigned long long* cycle_count,unsigned long long a,unsigned long long b){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:"

            "div  s8, s6, s7;"//full
            "sub  t4, s8, s8;"//full
            "add  s6, s6, t4;"

            "add  s4 ,s4, t4;"
            "addi s4 , s4 , 1;"
            //[c]
            //"%[d]"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "mv   s4 , zero;"
            "li   s5 , 500;"
            "mv   s6 , %[a];"
            "mv   s7 , %[b];"

            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "jal   zero, loop;"

        "term:"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : [a]"r"(a),[b]"r"(b)
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","cc"

    );

}
int main(int argc,char* argv[]){
    //unsigned long long op[8]={1,255,65535,16777215,4294967295,1099511627775,281474976710655,72057594037927935};
    //unsigned long long op[8]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
    unsigned long long op[8]={0x8,0x80,0x800,0x8000,0x80000,0x800000,0x8000000,0x80000000};
    unsigned long long busy_instr[8][8];
    unsigned long long busy_cycle[8][8];
    int i;
    int j;
    printf("div test\n");
    for( i=0;i<8;i++){
        for( j=0;j<8;j++){
            
            unsigned long long op1 = op[i];
            unsigned long long op2 = op[j];
//            unsigned long long busy_cycles = 0, busy_instrs = 0;
            busy_loop(&busy_instr[i][j], &busy_cycle[i][j],op1,op2);
          
        }
    }
    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            printf("i = %d;j = %d;op1 = busy_instr = %d;busy_cycle = %d\n",i,j,busy_instr[i][j],busy_cycle[i][j]);
        }

    }


    return 0;
}
