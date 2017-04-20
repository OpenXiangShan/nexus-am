#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"

uint32_t strlen(const char *str);
int strcmp(char *str1, int len, char *str2);
void i2str(char *str,int num);
void x2str(char *str, int num);

uint32_t strlen(const char *str) {	
	/*return lenth of str*/
	uint32_t count = 0;
	for(;str[count] != '\0';count++);
	return count;
}

int strcmp(char *str1, int len, char *str2) {	
	/*return 0 when str1 == str2 
	else return -1*/
	int i;
	for(i = 0; i < len; i++)
		if(str1[i] != str2[i])
			return -1;
	return 0;
}

void i2str(char *str,int num) {
	int count = 0;
	int pre_num = num;
	if(num == 0) {
		str[0] = '0';
		str[1] = '\0';
	}
	else {
		char sign = (uint32_t)num >> 31;
		if(sign == 1) {
			if(num == 0x80000000) {
				num = ~num;
				pre_num = num;
				count++;
				str[0] = '-';
				for(count = 0; num != 0; count++)
					num = num / 10;
				num = pre_num;
				int tail = count;
				str[count] = '\0';
				for(;num != 0; count--) {
					str[count] = (char)(num % 10 + '0');
					num = num / 10;
				}
				str[tail]++;
				return;
			}
			else {
				num = ~num + 1;
				pre_num = num;
				for(count = 0; num != 0; count++)
					num = num / 10;
				count++;
				str[0] = '-';
			}
		}
		if(sign == 0)
			for(count = 0; num != 0; count++)
				num = num / 10;
		num = pre_num;
		str[count] = '\0';
		for(;num != 0; count--) {
			str[count - 1] = (char)(num % 10 + '0');
			num = num / 10;
		}
	}
}

void x2str(char *str, int num) {
	int count = 0;
	int pre_num = num;
	if(num == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	for(; num != 0; count++) {
		num = (uint32_t)num >> 4;
	}
	num = pre_num;
	str[count] = '\0';
	for(;num != 0; count--) {
		switch((uint32_t)num % 16) {
			case 10:str[count - 1] = 'a';break;
			case 11:str[count - 1] = 'b';break;
			case 12:str[count - 1] = 'c';break;
			case 13:str[count - 1] = 'd';break;
			case 14:str[count - 1] = 'e';break;
			case 15:str[count - 1] = 'f';break;
			default :str[count - 1] = (char)((uint32_t)num % 16 + '0');
		}
		num = (uint32_t)num >> 4;
	}
	
}

#endif
