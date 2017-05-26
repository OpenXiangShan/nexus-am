#include"klib.h"
#include<stdarg.h>

static char* s_h=0;

char* printch(char ch,char* s);
char* printdec(unsigned int dec,int base,int width,char abs,char flagc,char* s);
int vprintdec(unsigned int dec,int base,int width,char abs,char flagc,char* s,int count);
//char* printstr(char* str,int width,char flagc,char* s);
char* printstr(char* str,char* s);
void myputc(char c){
  if(s_h==0)_putc(c);
  else *s_h++=c;
}


int vprintk(char* out,const char* fmt,va_list ap){
  s_h=out;
  int vargint=0;
  unsigned int varguint=0;
  char* vargpch=0;
  char vargch=0;
  char flagc=' ',abs='+';
  int base,width=-1;
  const char* pfmt=fmt;
  while(*pfmt){
    if(*pfmt == '%'){
      flagc=' ';
      width=-1;
      abs='+';
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
	case 'c':vargch=va_arg(ap,int);printch(vargch,out);break;
	case 'd':vargint=va_arg(ap,int);base=10;if(vargint<0){
		 //_putc('-');
		 abs='-';
		 varguint=-vargint;
	         }
		 else {varguint=vargint;}
		   //else if(flagc=='+'){flagc=' ';_putc('+');}
		   goto nump;
	//case 'o':vargint=va_arg(ap,unsigned int);base=8;goto nump;
	case 'u':varguint=va_arg(ap,unsigned int);base=10;goto nump;
	case 'x':
	case 'X':varguint=va_arg(ap,int);base=16;goto nump;
	case 'p':myputc('0');myputc('x');varguint=(long)va_arg(ap,void*);base=16;goto nump;
      nump:
	printdec(varguint,base,width,abs,flagc,out);break;
	case 's':vargpch=va_arg(ap,char*);printstr(vargpch,out);break;
	default:;
      }
      pfmt++;
    }
    else{
      out=printch(*pfmt++,out);
    }
  }
  if(out!=0)printch('\0',out);
  return 0;
}

int printk(const char* fmt,...){
  va_list ap;
  va_start(ap,fmt);
  int r=vprintk(0,fmt,ap);
  va_end(ap);
  return r;
}

/*
int sprintf(char* out,char* fmt,...){
  va_list ap;
  va_start(ap,fmt);
  int r=vprintk(out,fmt,ap);
  va_end(ap);
  return r;
}

int snprintf(char* out, size_t n, const char* fmt, ...){
  va_list ap;
  va_start(ap,fmt);
  int r=vprintk(out,fmt,ap);
  va_end(ap);
  if(strlen(out)>=n)*(out+n)='\0';
  return r;
}
*/

char* printch(char ch,char* s){
  //if(s==0)_putc(ch);
  //else *s++=ch;
  myputc(ch);
  return s;
}
//char* printdec(int dec,char* s){
char* printdec(unsigned int dec,int base,int width,char abs,char flagc,char* s){
  //int c=0;
  //if(abs=='-'||flagc=='+')c++;
  if(dec==0){
    myputc('0');
  }
  else vprintdec(dec,base,width,abs,flagc,s,0);
  //_putc(c+'0');
  return s;
}
int vprintdec(unsigned int dec,int base,int width,char abs,char flagc,char* s,int count){
  if(dec==0){
    if(flagc!='-'){
      width++;
      if(abs=='-')width--;
      while(width-->1){
        if(flagc!='+'){
          myputc(flagc);
        }
        else{
          myputc(' ');
        }
      }
      if(flagc=='+')myputc(abs);
      else if(abs=='-')myputc('-');
      else{
        if(width>0)myputc(flagc);
      }
      return 0;
    }
    else if(abs=='-'){myputc('-');width--;}
    return width;
  }
  int re;//vprintdec(dec/base,base,width-1,flagc,s+1);
  re=vprintdec(dec/base,base,width-1,abs,flagc,s,count+1);
  if(dec%base>9)myputc(dec%base+'a'-10);
  else myputc((char)(dec%base+'0'));
  if(flagc=='-'&&count==0){
    while(re-->0)myputc(' ');
  }
  return re;
}

char* printstr(char* str,char* s){
  while(*str){
    //_putc(*str);
    if(s==0)_putc(*str++);
    else*s++=*str++;
  }
  // *s++='\0';
  return s;
}
