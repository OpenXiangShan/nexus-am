#ifndef COMMON_H
#define COMMON_H

#include <am.h>

typedef u8 byte;
typedef u16 word;
typedef u32 dword;
typedef int bool;

#define true  1
#define false 0

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

// Binary Operations
bool common_bit_set(long long value, byte position);

// Byte Bit Operations
void common_set_bitb(byte *variable, byte position);
void common_unset_bitb(byte *variable, byte position);
void common_toggle_bitb(byte *variable, byte position);
void common_modify_bitb(byte *variable, byte position, bool set);

// Word Bit Operations
void common_set_bitw(word *variable, byte position);
void common_unset_bitw(word *variable, byte position);
void common_toggle_bitw(word *variable, byte position);
void common_modify_bitw(word *variable, byte position, bool set);

// Double Word Bit Operations
void common_set_bitd(dword *variable, byte position);
void common_unset_bitd(dword *variable, byte position);
void common_toggle_bitd(dword *variable, byte position);
void common_modify_bitd(dword *variable, byte position, bool set);

static inline void* memcpy(void *dest, const void *src, size_t n) {
  char *csrc = (char*)src, *cdest = (char*)dest;
  for (int i = 0; i < n; i ++) {
    cdest[i] = csrc[i];
  }
  return dest;
}

static inline int memcmp(const void *dest, const void *src, size_t n) {
  unsigned char *csrc = (unsigned char*)src, *cdest = (unsigned char*)dest;
  for (int i = 0; i < n; i ++) {
    if (csrc[i] < cdest[i]) return -1;
    if (csrc[i] > cdest[i]) return 1;
  }
  return 0;
}

#endif
