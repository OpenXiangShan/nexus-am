#include <stdarg.h>
#include <stdint.h>
#include <klib.h>

#include "wrapper.h"
#include "iommu_data_structures.h"
#include "iommu_registers.h"
#include "iommu_sys.h"
#include "openxiangshan_dmac.h"
#include "iommu_platform.h"
#include "iommu_cache.h"

#define IOMMU_REG_BASE          0x40200000UL
#define IOMMU_CUSTOM_MUX        0x40201000UL
#define IOPMP_HWCFG0            0x40100008UL
#define IOMMU_DDTP              (IOMMU_REG_BASE + 0x10)
#define IOMMU_IPSR              0x4020f000UL
#define IOMMU_ICVEC             0x4020f008UL
#define IOMMU_FCTL              (IOMMU_REG_BASE + 0x2b0)

#define PAGE_POOL_BASE          0x87110000UL
#define PAGE_POOL_SIZE          (2UL * 1024 * 1024)
#define SRC_PA                  0x98000000UL
#define DST_PA                  0x99000000UL
#define SRC_IOVA                0x1000UL
#define DST_IOVA                0x2000UL
#define DMA_BEAT_SIZE           32
#define DMA_TRANSFER_COUNT      3
#define DMA_TRANSFER_SIZE       (DMA_BEAT_SIZE * DMA_TRANSFER_COUNT)
#define DMA_TIMEOUT_CYCLES      1000000UL

static uintptr_t pool_next;
static const uintptr_t pool_end = PAGE_POOL_BASE + PAGE_POOL_SIZE;

static void *pool_alloc(unsigned long size, unsigned long alignment)
{
	uintptr_t addr = (pool_next + alignment - 1) & ~(alignment - 1);

	if (size == 0 || addr > pool_end || size > pool_end - addr)
		return NULL;
	pool_next = addr + size;
	memset((void *)addr, 0, size);
	return (void *)addr;
}

static int iommu_vprint(const char *fmt, va_list ap)
{
	return vprintf(fmt, ap);
}

static uint8_t iommu_mem_read(char *addr, char *data, uint32_t size)
{
	memcpy(data, addr, size);
	return 0;
}

static uint8_t iommu_mem_write(char *addr, char *data, uint32_t size)
{
	memcpy(addr, data, size);
	/* An 8-byte PTE write executes one cbo.flush for its cache line. */
	iommu_flush_range((uintptr_t)addr, size);
	return 0;
}

static uint64_t iommu_get_free_ppn(uint64_t num_ppn)
{
	void *addr;
	unsigned long size;

	if (num_ppn == 0 || num_ppn > PAGE_POOL_SIZE / 4096)
		return (uint64_t)-1;
	size = (unsigned long)num_ppn * 4096;
	addr = pool_alloc(size, 4096);
	if (!addr)
		return (uint64_t)-1;
	iommu_flush_range((uintptr_t)addr, size);
	return (uintptr_t)addr >> 12;
}

static uint64_t iommu_mm_alloc(int size)
{
	void *addr;

	if (size <= 0)
		return (uint64_t)-1;
	addr = pool_alloc((unsigned long)size, 16);
	return addr ? (uintptr_t)addr : (uint64_t)-1;
}

static void iommu_mm_free(uint64_t addr, int size)
{
	(void)addr;
	(void)size;
}

static struct memory_ops iommu_memory_ops = {
	.vprint = iommu_vprint,
	.read = iommu_mem_read,
	.write = iommu_mem_write,
	.get_free_ppn = iommu_get_free_ppn,
	.mm_alloc = iommu_mm_alloc,
	.mm_free = iommu_mm_free,
};

static void dma_data_init(uint8_t *data, int size)
{
	int i;

	for (i = 0; i < size; i++)
		data[i] = (uint8_t)(i ^ 0x5a);
}

static int dma_data_check(const uint8_t *src, const uint8_t *dst, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (src[i] != dst[i]) {
			printf("[IOMMU-TEST] data mismatch at %d: src=0x%x dst=0x%x\n",
			       i, src[i], dst[i]);
			return -1;
		}
	}
	return 0;
}

static int configure_iommu(uint32_t did, uint64_t *ddtp_out)
{
	uint64_t dc, pc, ddtp;

	ddtp = enable_iommu(DDT_3LVL);
	dc = add_device(did, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, IOHGATP_Bare, IOSATP_Sv48, PDTP_Bare,
			MSIPTP_Off, 1, 0, 0, 0, 0, &pc);
	if (dc == 0 || dc == (uint64_t)-1)
		return -1;

	printf("[IOMMU-TEST] ddtp=0x%lx dc=0x%lx pc=0x%lx\n", ddtp, dc, pc);
	if (iommu_ref_s1_only_page_mapping((void *)dc, SRC_IOVA, SRC_PA,
			DMA_TRANSFER_SIZE, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0))
		return -1;
	if (iommu_ref_s1_only_page_mapping((void *)dc, DST_IOVA, DST_PA,
			DMA_TRANSFER_SIZE, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0))
		return -1;

	*ddtp_out = ddtp;
	return 0;
}

int main(const char *args)
{
	const uint32_t did = 8;
	uint64_t ddtp;
	uint32_t ddtp_readback;
	uint32_t mux_before, mux_after;
	unsigned long start, cost, total_cost = 0;
	unsigned int transfer;
	int rc;

	(void)args;
	pool_next = PAGE_POOL_BASE;
	printf("[IOMMU-TEST] start: S-stage Sv48, DID=%u\n", did);

	if (libiommu_ref_init(&iommu_memory_ops)) {
		printf("[IOMMU-TEST] reference model init failed\n");
		return 1;
	}
	if (configure_iommu(did, &ddtp)) {
		printf("[IOMMU-TEST] page-table setup failed\n");
		return 1;
	}
	printf("[IOMMU-TEST] page-table setup success, pool used=0x%lx bytes\n",
	       pool_next - PAGE_POOL_BASE);

	dma_data_init((uint8_t *)SRC_PA, DMA_TRANSFER_SIZE);
	memset((void *)DST_PA, 0, DMA_TRANSFER_SIZE);
	iommu_flush_range(SRC_PA, DMA_TRANSFER_SIZE);
	iommu_flush_range(DST_PA, DMA_TRANSFER_SIZE);
	mb();

	/*
	 * Keep the data path bypassed until DDTP and the remaining IOMMU control
	 * registers are live.  Otherwise an early DMA request is correctly
	 * rejected by ACD with cause 256 (DDTP mode Off) instead of becoming a
	 * PTW request.
	 */
	writeq(IOMMU_DDTP, ddtp);
	writel(IOMMU_IPSR, 0x1);
	writel(IOMMU_ICVEC, 0x101);
	writel(IOMMU_FCTL, 0);
	writel(IOMMU_FCTL, 1);
	mb();

	ddtp_readback = readl(IOMMU_DDTP);
	if ((ddtp_readback & 0xf) == 0) {
		printf("[IOMMU-TEST] DDTP remains Off after programming: 0x%x\n",
		       ddtp_readback);
		return 1;
	}

	iommu_st_custom_init();
	iommu_st_custom_did_pid_enable();
	if (iommu_st_custom_set_did_pid_range(0, (unsigned long)-1, did, -1)) {
		printf("[IOMMU-TEST] DID address-range setup failed\n");
		return 1;
	}

	mux_before = readl(IOMMU_CUSTOM_MUX);
	writel(IOMMU_CUSTOM_MUX, mux_before & ~CUSTOM_MUX_BYPASS_EN);
	mux_after = readl(IOMMU_CUSTOM_MUX);
	printf("[IOMMU-TEST] hw_bypass_en: %u -> %u\n",
	       mux_before & 1, mux_after & 1);
	if (mux_after & CUSTOM_MUX_BYPASS_EN) {
		printf("[IOMMU-TEST] failed to enable IOMMU translation path\n");
		return 1;
	}
	printf("[IOMMU-TEST] IOPMP HWCFG0.enable=%u (0 means bypass)\n",
	       readl(IOPMP_HWCFG0) & 1);
	mb();

	for (transfer = 0; transfer < DMA_TRANSFER_COUNT; transfer++) {
		unsigned long offset = transfer * DMA_BEAT_SIZE;

		start = csr_read(CSR_MCYCLE);
		rc = openxiangshan_dmac_transfer((void *)(SRC_IOVA + offset),
				(void *)(DST_IOVA + offset), DMA_BEAT_SIZE);
		if (rc || openxiangshan_dmac_wait_for_complete(DMA_TIMEOUT_CYCLES)) {
			printf("[IOMMU-TEST] DMA %u/%u timeout/failure, ctrl=0x%lx\n",
			       transfer + 1, DMA_TRANSFER_COUNT,
			       readq(0x40003010UL));
			return 1;
		}
		cost = csr_read(CSR_MCYCLE) - start;
		total_cost += cost;
		printf("[IOMMU-TEST] DMA %u/%u completed in %lu cycles\n",
		       transfer + 1, DMA_TRANSFER_COUNT, cost);
	}
	printf("[IOMMU-TEST] %u DMA reads completed in %lu cycles\n",
	       DMA_TRANSFER_COUNT, total_cost);

	if (dma_data_check((const uint8_t *)SRC_PA,
			(const uint8_t *)DST_PA, DMA_TRANSFER_SIZE)) {
		printf("[IOMMU-TEST] TEST FAIL\n");
		return 1;
	}
	printf("[IOMMU-TEST] data check pass\n");
	printf("[IOMMU-TEST] TEST PASS\n");
	return 0;
}
