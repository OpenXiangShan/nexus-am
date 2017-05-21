#include "klib.h"

size_t strlen(const char *s) {
  size_t sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}

char* strcpy(char* dst,const char* src){
	char* ret;
	ret=dst;
	while((*dst++=*src++)!='\0');
	return ret;
}

void* memset(void* v,int c,size_t n){
	char* p;
	int m;
	p=v;
	m=n;
	while(--m>=0)*p++=c;
	return v;
}

void* memcpy(void* dst,const void* src,size_t n){
	const char* s;
	char* d;
	s=src+n;
	d=dst+n;
	while(n-->0)*--d=*--s;
	return dst;
}

int atoi(const char* s){
	const char* p=s;
	char c;
	int i=0;
	while((c=*p++)=='\0'){
		if(c>='0'&&c<='0')
			i=i*10+c-'0';
		else
			return -1;
	}
	return i;
}
