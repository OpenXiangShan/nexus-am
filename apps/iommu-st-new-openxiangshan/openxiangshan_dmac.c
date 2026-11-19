#include "iommu_platform.h"
#include "openxiangshan_dmac.h"

#define DMAC_BASE_ADDR 0x40003000

#define DMAC_SRC_ADDR_REG (DMAC_BASE_ADDR + 0x00)
#define DMAC_DST_ADDR_REG (DMAC_BASE_ADDR + 0x08)
#define DMAC_CTRL_REG (DMAC_BASE_ADDR + 0x10)

int openxiangshan_dmac_transfer(void *src, void *dst, int size)
{
	/* The simulation DMAC currently implements one 32-byte AXI beat. */
	if (size != 32)
		return -1;

	writeq(DMAC_SRC_ADDR_REG, (unsigned long)src);
	writeq(DMAC_DST_ADDR_REG, (unsigned long)dst);
	writeq(DMAC_CTRL_REG, 1);

	return 0;
}

int openxiangshan_dmac_wait_for_complete(unsigned long timeout_cycles)
{
	unsigned long start = csr_read(CSR_MCYCLE);

	while ((readq(DMAC_CTRL_REG) & 0x2) == 0) {
		if (csr_read(CSR_MCYCLE) - start > timeout_cycles)
			return -1;
	}
	return 0;
}
