#include <types.h>
int line,row = 0;

static inline int32_t syscall(int num, int check, uint32_t a1,uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	/*嵌入汇编代码，调用int $0x80*/
	int32_t ret = 0;
	asm volatile("int $0x80": "=a"(ret) :"a"(num),"d"(check),"c"(a1), "b"(a2), "D"(a3), "S"(a4));
	return ret;
}


int print(const char *str,int len){
	if(str[0] == '\\' && str[1] == 'n' && str[2] == '\0'){
		line += 1;
		row = 0;
	}
	int length = syscall(4,1,(uint32_t)str,len,line,row,0);
	row += length;
	if(row >= 80) {
		line += row / 80;
		row = row % 80;
	}
	if(length < 0 || length != len){
		while(1);
		return -1;
	}
	else
		return length;
}
