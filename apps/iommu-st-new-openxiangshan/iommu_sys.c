#include "iommu_platform.h"
#include "iommu_sys.h"

#define DEBUG 0

#define CUSTOM_BASE 0x40201000
static unsigned int iommu_st_custom_readl(unsigned long addr)
{
	return readl(CUSTOM_BASE + addr);
}

static void iommu_st_custom_writel(unsigned long addr, unsigned int val)
{
#if DEBUG
	printf("write 0x%x to 0x%lx\n", val, CUSTOM_BASE + addr);
#endif
	writel(CUSTOM_BASE + addr, val);
}

static unsigned long iommu_st_custom_get_slot_addr_start(int i)
{
	unsigned long hi, lo;

	lo = iommu_st_custom_readl(CUSTOM_MMIO_ADDR_RANGE_START + i * 8);
	hi = iommu_st_custom_readl(CUSTOM_MMIO_ADDR_RANGE_START + i * 8 + 4);

	return (lo + (hi << 32));
}
#if 0
static unsigned long iommu_st_custom_get_slot_addr_end(int i)
{
	unsigned long hi, lo;

	lo = iommu_st_custom_readl(CUSTOM_MMIO_ADDR_RANGE_END + i * 8);
	hi = iommu_st_custom_readl(CUSTOM_MMIO_ADDR_RANGE_END + i * 8 + 4);

	return (lo + (hi << 32));
}
#endif
static void iommu_st_custom_set_slot_addr_start(int i, unsigned long val)
{
	unsigned long hi, lo;

	lo = val << 32 >> 32;
	hi = val >> 32;

	iommu_st_custom_writel(CUSTOM_MMIO_ADDR_RANGE_START + i * 8, lo);
	iommu_st_custom_writel(CUSTOM_MMIO_ADDR_RANGE_START + i * 8 + 4, hi);
}

static void iommu_st_custom_set_slot_addr_end(int i, unsigned long val)
{
	unsigned long hi, lo;

	lo = val << 32 >> 32;
	hi = val >> 32;

	iommu_st_custom_writel(CUSTOM_MMIO_ADDR_RANGE_END + i * 8, lo);
	iommu_st_custom_writel(CUSTOM_MMIO_ADDR_RANGE_END + i * 8 + 4, hi);
}

static int iommu_st_custom_find_free_addr_slot(void)
{
	int i;

	for (i = 0; i < 8; i++) {
		if ((unsigned long)(-1) == iommu_st_custom_get_slot_addr_start(i))
			return i;
	}

	return -1;
}

static void iommu_st_custom_set_slot_did(int slot, int did)
{
	unsigned int val = did & CUSTOM_DID_MASK;
	iommu_st_custom_writel(CUSTOM_MMIO_DID + slot * 4, val);
}

static void iommu_st_custom_set_slot_pid(int slot, int pid)
{
	unsigned int val;

	val = iommu_st_custom_readl(CUSTOM_MMIO_PID + slot * 4);
	val |= CUSTOM_PID_V;
	val |= pid & CUSTOM_PID_MASK;

	iommu_st_custom_writel(CUSTOM_MMIO_PID + slot * 4, val);
}

int iommu_st_custom_set_did_pid_range(unsigned long from,
				      unsigned long to,
				      int did, int pid)
{
	int slot;

	slot = iommu_st_custom_find_free_addr_slot();
	if (slot == -1)
		return -1;

	iommu_st_custom_set_slot_addr_start(slot, from);
	iommu_st_custom_set_slot_addr_end(slot, to);
	iommu_st_custom_set_slot_did(slot, did);
	if (pid != -1)
		iommu_st_custom_set_slot_pid(slot, pid);

	return 0;
}

void iommu_st_custom_init(void)
{
	int i;

	for (i = 0; i < 8; i++) {
		iommu_st_custom_set_slot_addr_start(i, -1);
		iommu_st_custom_set_slot_addr_end(i, -1);
	}
}

void iommu_st_custom_did_pid_enable(void)
{
	unsigned int val = 0;

	val = iommu_st_custom_readl(CUSTOM_MMIO_MUX);

	val |= CUSTOM_MUX_DID_PID_EN;

	iommu_st_custom_writel(CUSTOM_MMIO_MUX, val);
}

void iommu_st_custom_set_mux(int mux)
{
	unsigned int val = 0;

	val = iommu_st_custom_readl(CUSTOM_MMIO_MUX);

	if (mux == CUSTOM_MUX_SELECT_BYPASS)
		val |= CUSTOM_MUX_BYPASS_EN;
	else if (mux == CUSTOM_MUX_SELECT_DMAC)
		val &= ~(1UL << CUSTOM_MUX_DMAC_SHIFT);
	else if (mux == CUSTOM_MUX_SELECT_AXI_TRAFFIC)
		val |= CUSTOM_MUX_AXI_TRAF_EN;

	iommu_st_custom_writel(CUSTOM_MMIO_MUX, val);
}
