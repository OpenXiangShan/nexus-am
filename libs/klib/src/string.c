#include "klib.h"

#ifndef __ISA_NATIVE__

size_t strlen(const char *s) {
  size_t sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}

char *strcpy(char* dst,const char* src) {
  char *ret;
  ret = dst;
  while((*dst++=*src++)!='\0');
  return ret;
}
char* strncpy(char* dst, const char* src, size_t n){
  char* ret;
  ret=dst;
  while(n-->0){
    *dst++=*src++;
  }
  return ret;
}

char* strcat(char* dst, const char* src){
  char* d=dst;
  while(*++dst!='\0');
  while((*dst++=*src++)!='\0');
  return d;
}
int strcmp(const char* s1, const char* s2){
  while(*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}
int strncmp(const char* s1, const char* s2, size_t n){
  while(--n>0&&*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}


void* memset(void* v,int c,size_t n){
  c &= 0xff;
  uint32_t c2 = (c << 8) | c;
  uint32_t c4 = (c2 << 16) | c2;
  uint64_t c8 = (((uint64_t)c4 << 16) << 16) | c4;
  int n_align = n & ~0xf;

  int i;
  // TODO: adjust 'v' to be 8-byte align to avoid unalign accesses on MIPS
  for (i = 0; i < n_align; i += 16) {
    *(uint64_t *)(v + i    ) = c8;
    *(uint64_t *)(v + i + 8) = c8;
  }

  for (; i < n; i ++) {
    ((char *)v)[i] = c;
  }

  return v;
}

void* memmove(void* dst,const void* src,size_t n){
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

void* memcpy(void* out, const void* in, size_t n) {
  // TODO: adjust 'dst' and 'src' to be 8-byte align to avoid unalign accesses on MIPS
  // I modify this implementation since align between dst and src is impossible, for example:
  //    dst = 2, src = 1, len = 13
  char *dst = (char *) out;
  char *src = (char *) in;
  while(n--) { *dst++ = *src++; }
  return out;
}
int memcmp(const void* s1, const void* s2, size_t n){
  return strncmp(s1,s2,n);
}

#endif
