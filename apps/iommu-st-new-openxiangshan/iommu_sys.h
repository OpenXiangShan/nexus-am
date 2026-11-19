#ifndef __IOMMU_ST_CUSTOM_H__
#define __IOMMU_ST_CUSTOM_H__

#define CUSTOM_MUX_SELECT_BYPASS 0
#define CUSTOM_MUX_SELECT_DMAC 1
#define CUSTOM_MUX_SELECT_AXI_TRAFFIC 2

#define CUSTOM_MUX_BYPASS_EN (1UL << 0)
#define CUSTOM_MUX_DID_PID_EN (1UL << 1)
#define CUSTOM_MUX_DMAC_SHIFT 2
#define CUSTOM_MUX_AXI_TRAF_EN (1UL << 2)
#define CUSTOM_MUX_MONITOR_EN (1UL << 3)

#define CUSTOM_DID_SHIFT 0
#define CUSTOM_DID_MASK 0x00ffffffUL

#define CUSTOM_PID_V (1UL << 20)
#define CUSTOM_PID_SHIFT 0
#define CUSTOM_PID_MASK 0x000fffffUL

#define CUSTOM_MMIO_MUX 0x0
#define CUSTOM_MMIO_DID 0x80
#define CUSTOM_MMIO_PID (0x80 + 0x20)
#define CUSTOM_MMIO_ADDR_RANGE_START (0x80 + 0x20 + 0x20)
#define CUSTOM_MMIO_ADDR_RANGE_END 0x100

int iommu_st_custom_set_did_pid_range(unsigned long from,
				      unsigned long to,
				      int did, int pid);
void iommu_st_custom_set_mux(int mux);
void iommu_st_custom_did_pid_enable(void);
void iommu_st_custom_init(void);

#endif
