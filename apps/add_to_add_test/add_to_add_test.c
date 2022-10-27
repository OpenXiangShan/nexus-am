#include <klib.h>

#define xstr(s) str(s)
#define str(s)  #s
#define OP1 72057594037927935
#define OP2 4294967295

#define EMPTY 0
#define FULL 1


#if EMPTY == 1
__attribute__((aligned(256)))
void empty_iadd(unsigned long long* instr_count, unsigned long long* cycle_count ){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, _ia_init;"

        "_ia_loop:"    
           // "add   t4, s6, s4;"  
            "add   t5, t4, s4;"

            "fence;"
            "addi s4 , s4 , 1;"
            "bleu s4 , s5 , _ia_loop;"
            "jal      zero, _ia_term;"

        "_ia_init:"
            "li   s4 , 0;"
            "li   s5 , 100;"
            "li   s6 , 0;"
            "li   s7 , 0;"
            "li   s8 , 0;"
            "li   t4 , 0;" 
            "fence;" 

            "csrr s9 , mcycle ;"
            "csrr s10, minstret;"

            "jal  zero, _ia_loop;"

        "_ia_term:"
            "fence;"
            "csrr s11, mcycle ;"
            "csrr t3 , minstret;"
            "fence;"

            "subw %[c], s11, s9;"
            "subw %[i], t3 , s10;"
            "fence;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","cc"

    );
}
#endif

#if FULL == 1
__attribute__((aligned(256)))
void busy_iadd(unsigned long long* instr_count, unsigned long long* cycle_count ){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, ia_init;"

        "ia_loop:"
            "add   t4, s6, s4;"
            //"add   s4, s4, s6;"
            "add   s4, t4, s6;"
            "addi  s4 , s4 , 1;"
            "bleu  s4 , s5 , ia_loop;"
            "jal   zero, ia_term;"

        "ia_init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0;"
            "li   s7 , 0;"
            "li   s8 , 0;"
            "li   t4 , 0;"


            "csrr s9 , mcycle ;"
            "csrr s10, minstret;"

            "jal  zero, ia_loop;"

        "ia_term:"
            "csrr s11, mcycle ;"
            "csrr t3 , minstret;"

            "subw %[c], s11, s9;"
            "subw %[i], t3 , s10;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","cc"

    );
}
#endif
int main(int argc,char* argv[]){

    unsigned long long iab_cycles[10] , iab_instrs [10];
    #if FULL ==1
    for(int i=0;i<10;i++){
        busy_iadd(&iab_instrs[i], &iab_cycles[i]);
    }
       
    #endif

    //unsigned long long iae_cycles = 0, iae_instrs = 0;
    #if EMPTY ==1    
    empty_iadd(&iae_instrs, &iae_cycles);
    #endif

 

//    unsigned long ia_instrs = iab_instrs - iae_instrs;
//    unsigned long ia_cycles = iab_cycles - iae_cycles;

    //printf("ia_busy_cycles %d\n",iab_cycles);
    //printf("ia_empty_cycles %d\n",iae_cycles);

    //printf("add to add instrs\t%d\tcycles\t%d\n", ia_instrs, ia_cycles); 
    for(int j =0;j<10;j++){
        printf("instrs %d ,cycles %d\n",iab_instrs[j],iab_cycles[j]);
    }


    return 0;
}
