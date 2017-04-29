#include<klib.h>
#include<stdarg.h>

void printch(char ch);
void printdec(int dec);
void printstr(char* str);

int printk(const char *fmt, ...){
	int vargint=0;
	char* vargpch=0;
	char vargch=0;
	
	const char* pfmt=0;
	va_list vp;

	va_start(vp,fmt);
	pfmt=fmt;

	while(*pfmt){
		if(*pfmt == '%'){
			switch(*(++pfmt)){
				case 'c':vargch=va_arg(vp,int);printch(vargch);break;
				case 'd':
				case 'i':vargint=va_arg(vp,int);printdec(vargint);break;
				case 's':vargpch=va_arg(vp,char*);printstr(vargpch);break;
				default:;
			}
			pfmt++;
		}
		else{
			printch(*pfmt++);
		}
	}
	va_end(vp);
	return 0;
}
void printch(char ch){
	_putc(ch);
}
void printdec(int dec){
	if(dec==0)return;
	printdec(dec/10);
	_putc((char)(dec%10+'0'));
}
void printstr(char* str){
	while(*str)
		_putc(*str++);
}
