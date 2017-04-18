#include <string.h>

extern int print(const char *str,int len);

void printf(const char *format,...){
	void **args = (void **)&format + 1;
	int count = 0;
	int i = 0;
	for(i = 0; format[i] != '\0'; i++) {
		if(strcmp((char *)&format[i], 2, "%s") == 0) {
			int len_temp = strlen(args[count]);
			print(args[count],len_temp);
			count++;
			i++;
		}
		else if(strcmp((char *)&format[i], 2, "%d") == 0) {
			char num[80];
			i2str(num,(int)args[count]);
			int len_temp = strlen(num);
			print(num,len_temp);
			count++;
			i++;
		}
		else if(strcmp((char *)&format[i], 2, "%x") == 0) {
			char num[80];
			x2str(num,(int)args[count]);
			int len_temp = strlen(num);
			print(num,len_temp);
			count++;
			i++;
		}
		else if(strcmp((char *)&format[i], 2, "%c") == 0) {
			print((void *)&args[count],1);
			count++;
			i++;
		}
		else if(format[i] == '\n') {
			print("\n",1);
		}
		else
			print((void *)&format[i],1);
	}
}
