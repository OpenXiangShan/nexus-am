#ifndef __IOMMU_TEST_CACHE_H__
#define __IOMMU_TEST_CACHE_H__

#include <stdint.h>

void cbo_cache_flush(unsigned long base);

static inline void iommu_flush_range(uintptr_t addr, unsigned long size)
{
	const uintptr_t line_size = 64;
	uintptr_t line = addr & ~(line_size - 1);
	uintptr_t end = addr + size;

	for (; line < end; line += line_size)
		cbo_cache_flush(line);
	__asm__ __volatile__("fence rw, rw" ::: "memory");
}

#endif
