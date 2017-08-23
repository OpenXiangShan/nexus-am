#include"klib.h"
#include<stdarg.h>

#ifndef __ISA_NATIVE__

char* printch(char ch,char** s);
int  printdec(unsigned int dec,int base,int width,char abs,char flagc,char** s);
int vprintdec(unsigned int dec,int base,int width,char abs,char flagc,char** s,int count);
char* printstr(char* str,char** s);
void myputc(char c,char** s_h){
  if(*s_h==0)_putc(c);
  else {**s_h=c;
	(*s_h)++;
  }
}


int vprintk(char* out,const char* fmt,va_list ap){
  unsigned int rewid=0;
  char** s_v=&out;
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
          }
          pfmt-=2;
          goto reswitch;
        case '*':width=va_arg(ap,int);goto reswitch;
	case 'c':vargch=va_arg(ap,int);printch(vargch,s_v);rewid++;break;
	case '%':printch('%',s_v);rewid++;break;
	case 'd':vargint=va_arg(ap,int);base=10;if(vargint<0){
		 abs='-';
		 varguint=-vargint;
	         }
		 else {varguint=vargint;}
		   goto nump;
	case 'u':varguint=va_arg(ap,unsigned int);base=10;goto nump;
	case 'x':
	case 'X':varguint=va_arg(ap,int);base=16;goto nump;
	case 'p':myputc('0',s_v);myputc('x',s_v);varguint=(long)va_arg(ap,void*);base=16;goto nump;
      nump:
	rewid+=printdec(varguint,base,width,abs,flagc,s_v);break;
	case 's':vargpch=va_arg(ap,char*);printstr(vargpch,s_v);rewid+=strlen(vargpch);break;
	default:;
      }
      pfmt++;
    }
    else{
      out=printch(*pfmt++,s_v);
      rewid++;
    }
  }
  if(out!=0){printch('\0',s_v);rewid++;}
  return rewid;
}

int printk(const char* fmt,...){
  va_list ap;
  va_start(ap,fmt);
  int r=vprintk(0,fmt,ap);
  va_end(ap);
  return r;
}

int sprintf(char* out,const char* fmt,...){
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

char* printch(char ch,char** s){
  //if(s==0)_putc(ch);
  //else *s++=ch;
  myputc(ch,s);
  return *s;
}
int printdec(unsigned int dec,int base,int width,char abs,char flagc,char** s){
  int rewid=0;
  if(abs=='-')rewid++;
  if(dec==0){
    if(width>0){
      if(flagc != '-'){
        while(--width>0){
	  if(flagc == '+')myputc(' ',s);
	  else myputc(flagc,s);
	}
      }
      myputc('0',s);
      if(flagc == '-'){
        while(--width>0){
	  myputc(' ',s);
	}
      }
    }
    else{
      myputc('0',s);
    }
    rewid++;
  }
  else vprintdec(dec,base,width,abs,flagc,s,0);
  while(dec>0){
	  dec=dec/base;
	  rewid++;
  }
  if(width>0&&rewid>width)rewid=width;
  return rewid;
}
int vprintdec(unsigned int dec,int base,int width,char abs,char flagc,char** s,int count){
  if(dec==0){
    if(flagc!='-'){
      width++;
      if(abs=='-')width--;
      while(width-->1){
        if(flagc!='+'){
          myputc(flagc,s);
        }
        else{
          myputc(' ',s);
        }
      }
      if(flagc=='+')myputc(abs,s);
      else if(abs=='-')myputc('-',s);
      else{
        if(width>0)myputc(flagc,s);
      }
      return width;
    }
    else if(abs=='-'){myputc('-',s);width--;}
    return width;
  }
  int re;//vprintdec(dec/base,base,width-1,flagc,s+1);
  re=vprintdec(dec/base,base,width-1,abs,flagc,s,count+1);
  if(dec%base>9)myputc(dec%base+'a'-10,s);
  else myputc((char)(dec%base+'0'),s);
  if(flagc=='-'&&count==0){
    while(re-->0)myputc(' ',s);
  }
  return re;
}

char* printstr(char* str,char** s){
  while(*str){
    //_putc(*str);
    //if(s==0)_putc(*str++);
    //else*s++=*str++;
    myputc(*str++,s);
  }
  //*s++='\0';
  return *s;
}

#endif
