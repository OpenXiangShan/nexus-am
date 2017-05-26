#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){
    cpu.esp -= 4;
    swaddr_write(cpu.esp,4,(op_src->val),R_SS);
    print_asm_template1();
}

make_instr_helper(i)
#if DATA_BYTE ==  2||DATA_BYTE ==4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"
