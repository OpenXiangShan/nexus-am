#include<klib.h>
int atoi(const char* s){
	const char* p=s;
	char flag;
	if(*p=='-'){
		flag='-';
		p++;
	}
	else flag='+';
	char c;
	int i=0;
	while((c=*p++)!='\0'){
		if(c>='0'&&c<='9')
			i=i*10+c-'0';
		else
			return -1;
	}
	if(flag=='-')return -i;
	else return i;
}
int abs(int x){
	//TODO
	return 0;
}
