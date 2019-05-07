#ifndef COMMON_H
#define COMMON_H

#include <am.h>
#include <amdev.h>
#include <klib.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef int bool;

#define true  1
#define false 0

// #define log(fmt, ...) printk("%s:%d: " fmt, __func__, __LINE__, ## __VA_ARGS__)
#define log(fmt, ...)

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

static inline bool common_bit_set(unsigned long value, byte position) { return value & (1L << position); }

static inline byte byte_pack(int expand[8]) {
  byte v = 0;
  v |= (!!expand[0] << 0);
  v |= (!!expand[1] << 1);
  v |= (!!expand[2] << 2);
  v |= (!!expand[3] << 3);
  v |= (!!expand[4] << 4);
  v |= (!!expand[5] << 5);
  v |= (!!expand[6] << 6);
  v |= (!!expand[7] << 7);
  return v;
}

static inline void byte_unpack(int expand[8], byte b) {
  expand[0] = (b >> 0) & 1;
  expand[1] = (b >> 1) & 1;
  expand[2] = (b >> 2) & 1;
  expand[3] = (b >> 3) & 1;
  expand[4] = (b >> 4) & 1;
  expand[5] = (b >> 5) & 1;
  expand[6] = (b >> 6) & 1;
  expand[7] = (b >> 7) & 1;
}

#endif
