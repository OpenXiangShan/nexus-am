#define CHANGE_PF \
		uint8_t least = result & 0xFF;\
		least ^= least>>4;\
		least ^= least>>2;\
		least ^= least>>1;\
		bool test_PF = !(least & 0x1);\
		test_PF?(cpu.EFLAGS.PF = 1):(cpu.EFLAGS.PF = 0);
#define CHANGE_ZF \
		if(result == 0)\
		cpu.EFLAGS.ZF = 1;\
		else\
		cpu.EFLAGS.ZF = 0;
#define CHANGE_SUB_CF \
		if((DATA_TYPE)op_dest->val < (DATA_TYPE)op_src->val)\
		cpu.EFLAGS.CF = 1;\
		else\
		cpu.EFLAGS.CF = 0;
#define CHANGE_SF \
		cpu.EFLAGS.SF = MSB(result);
#define CHANGE_ADD_OF \
		if(MSB(op_src->val) == MSB(op_dest->val) && MSB(result) != MSB(op_src->val))\
			cpu.EFLAGS.OF = 1;\
		else\
			cpu.EFLAGS.OF = 0;
#define CHANGE_SUB_OF \
		if((MSB(op_src->val) == 1 && MSB(op_dest->val) == 0 && MSB(result) == 1) ||(MSB(op_src->val) == 0 && MSB(op_dest->val) == 1 && MSB(result) == 0))\
			cpu.EFLAGS.OF = 1;\
		else\
			cpu.EFLAGS.OF = 0;
#define CHANGE_ADD_CF \
		if((DATA_TYPE)result < (DATA_TYPE)op_src->val || (DATA_TYPE)result < (DATA_TYPE)op_dest->val)\
			cpu.EFLAGS.CF = 1;\
		else\
			cpu.EFLAGS.CF = 0;
