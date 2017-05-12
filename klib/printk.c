#include<am.h>
#include<klib.h>
#include<stdarg.h>

char* printch(char ch,char* s);
char* printdec(int dec,char* s);
char* printstr(char* str,char* s);

int printk(const char *fmt, ...){
	//return sprintk(0,fmt,...);
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
				case 'c':vargch=va_arg(vp,int);printch(vargch,0);break;
				case 'd':
				case 'i':vargint=va_arg(vp,int);printdec(vargint,0);break;
				case 's':vargpch=va_arg(vp,char*);printstr(vargpch,0);break;
				default:;
			}
			pfmt++;
		}
		else{
			printch(*pfmt++,0);
		}
	}
	va_end(vp);
	return 0;
}
int sprintk(char* out,char* fmt,...){
	int vargint=0;
	char* vargpch=0;
	char vargch=0;
	
	char* pfmt=0;
	va_list vp;

	va_start(vp,fmt);
	pfmt=fmt;

	while(*pfmt){
		if(*pfmt == '%'){
			switch(*(++pfmt)){
				case 'c':vargch=va_arg(vp,int);out=printch(vargch,out);break;
				case 'd':
				case 'i':vargint=va_arg(vp,int);out=printdec(vargint,out);break;
				case 's':vargpch=va_arg(vp,char*);out=printstr(vargpch,out);break;
				default:;
			}
			pfmt++;
		}
		else{
			out=printch(*pfmt++,out);
		}
	}
	va_end(vp);
	*out='\0';
	return 0;

}
char* printch(char ch,char* s){
	if(s==0)_putc(ch);
	else *s++=ch;
	return s;
}
char* printdec(int dec,char* s){
	if(dec==0)return s;
	//_putc('0');
	s=printdec(dec/10,s);
	if(s==0)_putc((char)(dec%10+'0'));
	else *s++=(char)(dec%10+'0');
	return s;
}
char* printstr(char* str,char* s){
	while(*str){
		//_putc(*str);
		if(s==0)_putc(*str++);
		else*s++=*str++;
	}
	//*s++='\0';
	return s;
}
