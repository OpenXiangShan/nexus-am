#ifndef __COMMON_H__
#define __COMMON_H__

#include <arch.h> // achitectural dependent definitions

// integer types (u8, i8, u16, i16, etc.)
// _RegSet structure

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char uchar;

// C++ compatibility
#ifdef __cplusplus
  #define __H_BEGIN__  extern "C" {
  #define __H_END__    }
#else
  #define __H_BEGIN__
  #define __H_END__
#endif

#endif
