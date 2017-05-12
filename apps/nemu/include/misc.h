#ifndef __LIB_MISC_H__
#define __LIB_MISC_H__

#include "common.h"

inline static void memcpy_with_mask(void *dest, const void *src, size_t len, uint8_t *mask) {
	int l;
	for(l = 0; l < len; l ++) {
		if(mask[l]) {
			((uint8_t *)dest)[l] = ((uint8_t *)src)[l];
		}
	}
}

#endif
