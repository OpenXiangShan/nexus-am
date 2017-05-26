#include <klib.h>

size_t strlen(const char *s) {
	assert(s);
	size_t sz = 0;
	for (; *s; s ++) {
	sz ++;
  	}
  	return sz;
}

char* strcpy(char* dst,const char* src){
	assert(src&&dst);
	char* ret;
	ret=dst;
	while((*dst++=*src++)!='\0');
	return ret;
}
char* strncpy(char* dst, const char* src, size_t n){
	assert(src&&dst);
	char* ret;
	ret=dst;
	while(n-->0){
		*dst++=*src++;
	}
	return ret;
}

char* strcat(char* dst, const char* src){
	assert(dst&&src);
	char* d=dst;
	while(*++dst!='\0');
	while((*dst++=*src++)!='\0');
	return d;
}
int strcmp(const char* s1, const char* s2){
	assert(s1&&s2);
	while(*s1&&*s1==*s2)s1++,s2++;
	return (int)(*s1-*s2);
}
int strncmp(const char* s1, const char* s2, size_t n){
	assert(s1&&s2);
	while(--n>0&&*s1&&*s1==*s2)s1++,s2++;
	return (int)(*s1-*s2);
}
char* s_last;
char* strtok(char* s,char* delim){
	if(s==NULL)s=s_last;
	if(s==NULL)return NULL;
	char* tok=s;
	while(*s!='\0'){
		char* dm=delim;
		while(*dm!='\0'){
			if(*s==*dm){
				*s='\0';
				s_last=s+1;
				break;
			}
			dm++;
		}
		s++;
	}
	return tok;
}



void* memset(void* v,int c,size_t n){
	assert(v);
	char* p;
	int m;
	p=v;
	m=n;
	while(--m>=0)*p++=c;
	return v;
}

void* memmove(void* dst,const void* src,size_t n){
	assert(dst&&src);
	const char* s;
	char* d;
	if(src+n>dst&&src<dst){
		s=src+n;
		d=dst+n;
		while(n-->0)*--d=*--s;
	}
	else{
		s=src;
		d=dst;
		while(n-->0)*d++=*s++;
	}
	return dst;
}
void* memcpy(void* dst, const void* src, size_t n){
	memmove(dst,src,n);
	return dst;
}
int memcmp(const void* s1, const void* s2, size_t n){
	return strncmp(s1,s2,n);
}

