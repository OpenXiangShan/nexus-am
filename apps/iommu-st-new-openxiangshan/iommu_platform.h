#ifndef __IOMMU_TEST_PLATFORM_H__
#define __IOMMU_TEST_PLATFORM_H__

#include <stdint.h>
#include <klib.h>
#include <riscv.h>
#include <csr.h>

static inline uint32_t readl(uintptr_t addr)
{
	return *(volatile uint32_t *)addr;
}

static inline uint64_t readq(uintptr_t addr)
{
	return *(volatile uint64_t *)addr;
}

static inline void writel(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
}

static inline void writeq(uintptr_t addr, uint64_t value)
{
	*(volatile uint64_t *)addr = value;
}

static inline void mb(void)
{
	__asm__ __volatile__("fence iorw, iorw" ::: "memory");
}

#endif
