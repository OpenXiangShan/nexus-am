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
	if(x<0)return -x;
	return x;
}
ulong time(){
	return _uptime();
}
unsigned long next=1;
void srand(unsigned int seed){
	next=seed;
}
int rand(){
	next=next*1103515245+12345;
	return ((unsigned)(next/65536)%32768);
}

