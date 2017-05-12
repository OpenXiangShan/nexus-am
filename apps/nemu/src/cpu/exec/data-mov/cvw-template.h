#include "cpu/exec/template-start.h"

#define instr cvw

make_helper(concat(cvw_none_,SUFFIX)){
	if(DATA_BYTE == 2)
	{
		if((DATA_TYPE_S)REG(R_AX)< 0)
			REG(R_DX) = 0xFFFF;
		else
			REG(R_DX) = 0x0;
		print_asm("%s" ,"cltd");
	}
	else
	{
		if((DATA_TYPE_S)cpu.eax < 0)
			cpu.edx = 0xFFFFFFFF;

		else
			cpu.edx = 0x0;
		print_asm("%s" ,"cltd");
	}
	return 1;
}

make_helper(concat(cbw_none_,SUFFIX)){
	if(DATA_BYTE == 2)
	{
		REG(R_AX) = (DATA_TYPE_S)(REG(R_AL));
		print_asm("%s" ,"cwtl");
	}
	else
	{
		REG(R_EAX) = (DATA_TYPE_S)(REG(R_AX));
		print_asm("%s" ,"cwtl");
	}
	return 1;
}

#include "cpu/exec/template-end.h"

