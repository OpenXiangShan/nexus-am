#include <cpu.h>

char *One_Byte_Op_Map[16][16] = {
/*0x0	0x1	0x2	0x3	0x4	0x5	0x6	0x7	0x8	0x9	0xa	0xb	0xc	0xd	0xe	0xf*/
{"add_EG_b","add_EG_v","add_GE_b","add_GE_v","add_AI_b","add_AI_v","push_ES","pop_ES","or_EG_b","or_EG_v","or_GE_b","or_GE_v","or_AI_b","or_AI_v","push_CS","2_byte_escape"},
{"adc_EG_b","adc_EG_v","adc_GE_b","adc_GE_v","adc_AI_b","adc_AI_v","push_SS","pop_SS","sbb_EG_b","sbb_EG_v","sbb_GE_b","sbb_GE_v","sbb_AI_b","sbb_AI_v","push_DS","push_DS"},
{"and_EG_b","and_EG_v","and_GE_b","and_GE_v","and_AI_b","and_AI_v","seg_ES","daa","sub_EG_b","sub_EG_v","sub_GE_b","sub_GE_v","sub_AI_b","sub_AI_v","seg_CS","das"},
{"xor_EG_b","xor_EG_v","xor_GE_b","xor_GE_v","xor_AI_b","xor_AI_v","seg_SS","add","cmp_EG_b","cmp_EG_v","cmp_GE_b","cmp_GE_v","cmp_AI_b","cmp_AI_v","seg_CS","aas"},
{"inc_A_v","inc_C_v","inc_D_v","inc_B_v","inc_SP_v","inc_BP_v","inc_SI_v","inc_DI_v","dec_A_v","dec_C_v","dec_D_v","dec_B_v","dec_SP_v","dec_BP_v","dec_SI_v","dec_DI_v"}
};
