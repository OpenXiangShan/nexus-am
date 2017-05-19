#include<am.h>
#include<klib.h>
#include<stdarg.h>

char* printch(char ch,char* s);
char* printdec(int dec,int base,int width,char flagc,char* s);
int vprintdec(int dec,int base,int width,char flagc,char* s);
//char* printstr(char* str,int width,char flagc,char* s);
char* printstr(char* str,char* s);

int vprintk(char* out,const char* fmt,va_list ap){
	int vargint=0;
	char* vargpch=0;
	char vargch=0;
	char flagc=' ';
	int base,width=-1;
	const char* pfmt=fmt;
	while(*pfmt){
		if(*pfmt == '%'){
			flagc=' ';
			width=-1;
		reswitch:
			//flagc=' ';
			//width=-1;
			switch(*(++pfmt)){
				case '-':flagc='-';goto reswitch;
				case '0':flagc='0';goto reswitch;
				case ' ':flagc=' ';goto reswitch;
				case '+':flagc='+';goto reswitch;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					width=0;
					char ch;
					while( (ch=*pfmt++)>='0'&&ch<='9'){
						width=width*10+ch-'0';
						//_putc(width+'0');
					}
					pfmt-=2;
					//_putc(*(pfmt));
					goto reswitch;
				case '*':width=va_arg(ap,int);goto reswitch;
				//case '.':if(width<0)width=0;goto reswitch;
				case 'c':vargch=va_arg(ap,int);printch(vargch,0);break;
				case 'd':vargint=va_arg(ap,int);base=10;if(vargint<0){
						 _putc('-');
						 vargint=-vargint;
					 }
					 else if(flagc=='+'){flagc=' ';_putc('+');}
					 goto nump;
				//case 'o':vargint=va_arg(ap,unsigned int);base=8;goto nump;
				case 'u':vargint=va_arg(ap,unsigned int);base=10;goto nump;
				case 'x':
				case 'X':vargint=va_arg(ap,int);base=16;goto nump;
				case 'p':_putc('0');_putc('x');vargint=(long)va_arg(ap,void*);base=16;goto nump;
			nump:
					 printdec(vargint,base,width,flagc,0);break;
				case 's':vargpch=va_arg(ap,char*);printstr(vargpch,0);break;
				default:;
			}
			pfmt++;
		}
		else{
			printch(*pfmt++,0);
		}
	}
	return 0;
}

int printk(const char* fmt,...){
	va_list ap;
	va_start(ap,fmt);
	int r=vprintk(0,fmt,ap);
	va_end(ap);
	return r;
}

int sprintf(char* out,char* fmt,...){
	va_list ap;
	va_start(ap,fmt);
	int r=vprintk(out,fmt,ap);
	va_end(ap);
	return r;
}
char* printch(char ch,char* s){
	if(s==0)_putc(ch);
	else *s++=ch;
	return s;
}
//char* printdec(int dec,char* s){
char* printdec(int dec,int base,int width,char flagc,char* s){
	if(dec==0){
		_putc('0');
	}
	else vprintdec(dec,base,width,flagc,s);
	return s;
}
int vprintdec(int dec,int base,int width,char flagc,char* s){
	/*if(dec<0){
		_putc('-');
		vprintdec(-dec,s);
		return s;
	}*/
	//_putc(flagc);
	if(dec==0){
		if(flagc!='-'){
			//_putc(width+'0');
			while(width-->0){
				if(s==0)_putc(flagc);
				else *s++=flagc;
			}
			return 0;
		}
		return width;
	}
	//_putc(width+'0');
	//assert(base==16);
	//_putc((char)(dec%base+'0'));
	int re;//vprintdec(dec/base,base,width-1,flagc,s+1);
	if(s==0){
		re=vprintdec(dec/base,base,width-1,flagc,s);
		if(flagc=='-'){
			while(re-->0)_putc(' ');
		}
		if(dec%base>9)_putc(dec%base+'a'-10);
		else _putc((char)(dec%base+'0'));
	}
	else {
		re=vprintdec(dec/base,base,width-1,flagc,s+1);
		if(flagc=='-'){
			while(re-->0)*s++=' ';
		}
		if(dec%base>9)*s=(char)(dec%base+'a'-10);
		else *s=(char)(dec%base+'0');
	}
	/*
	if(flagc=='-'){
		while(re-->0){
			if(s==0)_putc(' ');
			else *s++=' ';
		}
	}*/
	return re;
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
