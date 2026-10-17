#include <string.h>
#include <stdint.h>
#include "wrapper.h"
#include "utils.h"
#include "iommu_data_structures.h"
#include "iommu_utils.h"
#include "iommu_registers.h"
#include "iommu_translate.h"
#include "tables_api.h"
#include "queue.h"

#define get_field(reg, mask) (((reg) & \
                 (unsigned long)(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(unsigned long)(mask)) | \
                 (((unsigned long)(val) * ((mask) & ~((mask) << 1))) & \
                 (unsigned long)(mask)))

#define _PAGE_NAPOT (1UL << 63)
#define PAGE_SHIFT       12
#define PAGE_SIZE        (1UL << PAGE_SHIFT)
#define _PAGE_PFN_SHIFT 10
#define _PAGE_PFN_MASK  (0x3FFFFFFFFFFC00ULL)
#define GENMASK(a, b) (a > b ? (((1UL << (a+1)) - 1) >> (b) << (b)) : (((1UL << (b+1)) - 1) >> (a) << (a)))
enum napot_order {
	NAPOT_8K = 1,
	NAPOT_16K,
	NAPOT_32K,
	NAPOT_64K,
	NAPOT_MAX
};
#define for_each_napot_order(order) \
	for (order = NAPOT_8K; order < NAPOT_MAX; order++)

static uint64_t __build_s1_s2_pte_and_get_leaf(iosatp_t satp, iohgatp_t iohgatp, unsigned long iova,
                                               unsigned long pa, int8_t add_level,
                                               int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                               int8_t G, int8_t A, int8_t D, int8_t PBMT,
                                               unsigned long *s1_pte, unsigned long *s2_pte)
{
    unsigned long gpa;
    gpte_t gpte;
    pte_t pte;

    pte.raw = 0;
    pte.V = V;
    pte.R = R;
    pte.W = W;
    pte.X = X;
    pte.U = U;
    pte.G = G;
    pte.A = A;
    pte.D = D;
    pte.PBMT = PBMT;
    pte.PPN = get_free_gppn(16, iohgatp.raw);
    gpa = pte.PPN * PAGESIZE;

    gpte.raw = 0;
    gpte.V = V;
    gpte.R = R;
    gpte.W = W;
    gpte.X = X;
    gpte.U = 1;
    gpte.G = G;
    gpte.A = A;
    gpte.D = D;
    gpte.PBMT = PBMT;
    gpte.PPN = pa >> PAGESHIFT;

    *s2_pte = add_g_stage_pte(iohgatp, gpa, gpte, 0);

    return add_vs_stage_pte(satp, iova, pte, add_level, iohgatp, 0);
}

static uint64_t __build_s1_s2_pte(iosatp_t satp, iohgatp_t iohgatp, unsigned long iova,
                                  unsigned long pa, int8_t add_level,
                                  int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                  int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    unsigned long gpa;
    gpte_t gpte;
    pte_t pte;

    pte.raw = 0;
    pte.V = V;
    pte.R = R;
    pte.W = W;
    pte.X = X;
    pte.U = U;
    pte.G = G;
    pte.A = A;
    pte.D = D;
    pte.PBMT = PBMT;
    pte.PPN = get_free_gppn(16, iohgatp.raw);
    gpa = pte.PPN * PAGESIZE;

    gpte.raw = 0;
    gpte.V = V;
    gpte.R = R;
    gpte.W = W;
    gpte.X = X;
    gpte.U = 1;
    gpte.G = G;
    gpte.A = A;
    gpte.D = D;
    gpte.PBMT = PBMT;
    gpte.PPN = pa >> PAGESHIFT;

    add_g_stage_pte(iohgatp, gpa, gpte, 0);

    return add_vs_stage_pte(satp, iova, pte, add_level, iohgatp, 0);
}

uint64_t build_mrif_pte(void *dc_addr, unsigned long msi_gpa, unsigned long *mrif_addr,
			unsigned long notice_msi_addr, unsigned long notice_msi_data)
{
    device_context_t *DC;
    msipte_t msi_pte = { 0 };
    uint64_t msi_ptep, addr;
    uint64_t no;

    *mrif_addr = (unsigned long)my_alloc(4096);

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    msi_ptep = DC->msiptp.PPN << PAGE_SHIFT;
    no = _pext_u64(msi_gpa >> PAGE_SHIFT, DC->msi_addr_mask.raw);
    if (no >= 256)
        return -1;

    msi_pte.mrif.C = 0;
    msi_pte.mrif.M = 1;
    msi_pte.mrif.V = 1;
    msi_pte.mrif.MRIF_ADDR_55_9 = (*mrif_addr) >> 9;
    msi_pte.mrif.NPPN = notice_msi_addr >> PAGE_SHIFT;
    msi_pte.mrif.N90 = notice_msi_data & 0x3ffUL;
    msi_pte.mrif.N10 = notice_msi_data >> 10;

    addr = msi_ptep | (no * sizeof(msipte_t));

    my_print("wirte 0x%lx to 0x%lx\n", msi_pte.raw[0], addr);
    my_print("wirte 0x%lx to 0x%lx\n", msi_pte.raw[1], addr + 8);
    write_memory_test((char *)&msi_pte, addr, sizeof(msipte_t));

    return 0;
}

uint64_t build_basic_msi_pte(void *dc_addr, unsigned long msi_gpa, unsigned long msi_hpa)
{
    device_context_t *DC;
    msipte_t msi_pte = { 0 };
    uint64_t msi_ptep, addr;
    uint64_t no;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    msi_ptep = DC->msiptp.PPN << PAGE_SHIFT;
    no = _pext_u64(msi_gpa >> PAGE_SHIFT, DC->msi_addr_mask.raw);
    if (no >= 256)
        return -1;

    msi_pte.translate_rw.C = 0;
    msi_pte.translate_rw.M = 3;
    msi_pte.translate_rw.V = 1;
    msi_pte.translate_rw.PPN = msi_hpa >> PAGE_SHIFT;

    addr = msi_ptep | (no * sizeof(msipte_t));

    my_print("wirte 0x%lx to 0x%lx\n", msi_pte.raw[0], addr);
    my_print("wirte 0x%lx to 0x%lx\n", msi_pte.raw[1], addr + 8);
    write_memory_test((char *)&msi_pte, addr, sizeof(msipte_t));

    return 0;
}

uint64_t build_vs_stage_pte(void *dc_addr, unsigned long iova,
                            unsigned long gpa, int8_t add_level,
                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                            int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    device_context_t *DC;
    iohgatp_t iohgatp;
    iosatp_t satp;
    pte_t pte;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    iohgatp = DC->iohgatp;
    satp = DC->fsc.iosatp;

    pte.raw = 0;
    pte.V = V;
    pte.R = R;
    pte.W = W;
    pte.X = X;
    pte.U = U;
    pte.G = G;
    pte.A = A;
    pte.D = D;
    pte.PBMT = PBMT;
    pte.PPN = gpa >> PAGESHIFT;

    return add_vs_stage_pte(satp, iova, pte, add_level, iohgatp, 0);
}

uint64_t build_s1_s2_pte_and_get_leaf(void *dc_addr, unsigned long iova,
                                      unsigned long pa, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT,
                                      unsigned long *s1_pte, unsigned long *s2_pte)
{
    device_context_t *DC;
    iohgatp_t iohgatp;
    iosatp_t satp;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    iohgatp = DC->iohgatp;
    satp = DC->fsc.iosatp;

    return __build_s1_s2_pte_and_get_leaf(satp, iohgatp, iova, pa, add_level,
                                          V, R, W, X, U, G, A, D, PBMT,
                                          s1_pte, s2_pte);
}

uint64_t build_s1_s2_pte(void *dc_addr, unsigned long iova,
                         unsigned long pa, int8_t add_level,
                         int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                         int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    device_context_t *DC;
    iohgatp_t iohgatp;
    iosatp_t satp;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    iohgatp = DC->iohgatp;
    satp = DC->fsc.iosatp;

    return __build_s1_s2_pte(satp, iohgatp, iova, pa, add_level, V, R, W, X, U, G, A, D, PBMT);
}

uint64_t build_s1_s2_pte_pc(void *pc_addr, void *dc_addr, unsigned long iova,
                            unsigned long pa, int8_t add_level,
                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                            int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    device_context_t *DC;
    process_context_t *PC;
    iosatp_t satp;
    iohgatp_t iohgatp;

    PC = (process_context_t *)pc_addr;
    if (!PC)
        return -1;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;

    satp = PC->fsc.iosatp;
    iohgatp = DC->iohgatp;

    return __build_s1_s2_pte(satp, iohgatp, iova, pa, add_level, V, R, W, X, U, G, A, D, PBMT);
}

uint64_t build_s2_only_pte(void *dc_addr, unsigned long gpa,
                           unsigned long pa, int8_t add_level,
                           int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                           int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    device_context_t *DC;
    iohgatp_t iohgatp;
    gpte_t gpte;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;
    iohgatp = DC->iohgatp;

    gpte.raw = 0;
    gpte.V = V;
    gpte.R = R;
    gpte.W = W;
    gpte.X = X;
    gpte.U = U;
    gpte.G = G;
    gpte.A = A;
    gpte.D = D;
    gpte.PBMT = PBMT;
    gpte.PPN = pa >> PAGESHIFT;

    return add_g_stage_pte(iohgatp, gpa, gpte, add_level);
}

static uint64_t __build_s1_only_pte(iosatp_t satp, unsigned long va,
                           unsigned long pa, int8_t add_level,
                           int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                           int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    pte_t pte;

    pte.raw = 0;
    pte.V = V;
    pte.R = R;
    pte.W = W;
    pte.X = X;
    pte.U = U;
    pte.G = G;
    pte.A = A;
    pte.D = D;
    pte.PBMT = PBMT;
    pte.PPN = pa >> PAGESHIFT;

    return add_s_stage_pte(satp, va, pte, add_level, 0);
}

uint64_t build_s1_only_pte_pc(void *pc_addr, unsigned long va,
                              unsigned long pa, int8_t add_level,
                              int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                              int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    process_context_t *PC;
    iosatp_t satp;

    PC = (process_context_t *)pc_addr;
    if (!PC)
        return -1;
    satp = PC->fsc.iosatp;

    return __build_s1_only_pte(satp, va, pa, add_level, V, R, W, X, U, G, A, D, PBMT);
}

uint64_t build_s1_only_pte(void *dc_addr, unsigned long va,
                           unsigned long pa, int8_t add_level,
                           int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                           int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    device_context_t *DC;
    iosatp_t satp;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;

    if (DC->tc.PDTV)
        return -1;
    satp = DC->fsc.iosatp;

    return __build_s1_only_pte(satp, va, pa, add_level, V, R, W, X, U, G, A, D, PBMT);
}

static int get_napot_order(unsigned int size)
{
	int order;

	for_each_napot_order(order) {
		if ((1UL << (order + PAGE_SHIFT)) == size)
			return order;
	}

	return -1;
}

uint64_t build_s1_only_pte_napot(void *dc_addr, unsigned long va,
                                 unsigned long pa, int8_t add_level,
                                 int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                 int8_t G, int8_t A, int8_t D, int8_t PBMT, int page_size)
{
    device_context_t *DC;
    iosatp_t satp;
    unsigned long napot_bits, napot_mask;
    uint64_t* pte;
    int order;

    DC = (device_context_t *)dc_addr;
    if (!DC)
        return -1;

    if (DC->tc.PDTV)
        return -1;
    satp = DC->fsc.iosatp;

    order = get_napot_order(page_size);
    if (order == -1) {
        my_print("%s -- Unsupport napot size\n", __FUNCTION__);
        return -1;
    }

    napot_bits = 1UL << (order - 1 + _PAGE_PFN_SHIFT);
    napot_mask = GENMASK(order - 1 + _PAGE_PFN_SHIFT, _PAGE_PFN_SHIFT);

    pte = (uint64_t *)__build_s1_only_pte(satp, va, pa, add_level, V, R, W, X, U, G, A, D, PBMT);

    *pte = (*pte & (~napot_mask)) | ((napot_bits) & napot_mask) | _PAGE_NAPOT;

    return 0;
}

int8_t iommu_ref_s1_only_page_mapping_napot(void *dc_addr, unsigned long va,
                                            unsigned long pa, int size, int page_size,
                                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                            int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    uint64_t offset;

    for (offset = 0; offset < size; offset += 4096) {
        uint64_t vaddr = va + offset;
        uint64_t paddr = pa + offset;

        if (-1 == build_s1_only_pte_napot(dc_addr, vaddr, paddr, 0,
                                          V, R, W, X, U, G, A, D, PBMT, page_size))
            return -1;
    }

    return 0;
}

int8_t iommu_ref_s1_only_page_mapping(void *dc_addr, unsigned long va,
                                      unsigned long pa, int size, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    int page_size;
    uint64_t offset;

    page_size = PAGESIZE << (add_level * 9);

    for (offset = 0; offset < size; offset += page_size) {
        uint64_t vaddr = va + offset;
        uint64_t paddr = pa + offset;
        
        if (-1 == build_s1_only_pte(dc_addr, vaddr, paddr, add_level,
                                    V, R, W, X, U, G, A, D, PBMT))
            return -1;
    }

    return 0;
}

int8_t iommu_ref_s2_only_page_mapping(void *dc_addr, unsigned long va,
                                      unsigned long pa, int size, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    int page_size;
    uint64_t offset;

    page_size = PAGESIZE << (add_level * 9);

    for (offset = 0; offset < size; offset += page_size) {
        uint64_t vaddr = va + offset;
        uint64_t paddr = pa + offset;
        
        if (-1 == build_s2_only_pte(dc_addr, vaddr, paddr, add_level,
                                    V, R, W, X, U, G, A, D, PBMT))
            return -1;
    }

    return 0;
}

int8_t iommu_ref_s1_s2_page_mapping_and_get_leaf(void *dc_addr, unsigned long va,
                                                 unsigned long pa, int size, int8_t add_level,
                                                 int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                                 int8_t G, int8_t A, int8_t D, int8_t PBMT,
                                                 unsigned long *s1_pte, unsigned long *s2_pte)
{
    int page_size;
    uint64_t offset;

    page_size = PAGESIZE << (add_level * 9);

    for (offset = 0; offset < size; offset += page_size) {
        uint64_t vaddr = va + offset;
        uint64_t paddr = pa + offset;

        *s1_pte =  build_s1_s2_pte_and_get_leaf(dc_addr, vaddr, paddr, add_level,
                                                V, R, W, X, U, G, A, D, PBMT,
                                                s1_pte, s2_pte);
        if (*s1_pte == -1)
            return -1;
    }

    return 0;
}

int8_t iommu_ref_s1_s2_page_mapping(void *dc_addr, unsigned long va,
                                    unsigned long pa, int size, int8_t add_level,
                                    int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                    int8_t G, int8_t A, int8_t D, int8_t PBMT)
{
    int page_size;
    uint64_t offset;

    page_size = PAGESIZE << (add_level * 9);

    for (offset = 0; offset < size; offset += page_size) {
        uint64_t vaddr = va + offset;
        uint64_t paddr = pa + offset;
        
        if (-1 == build_s1_s2_pte(dc_addr, vaddr, paddr, add_level,
                                  V, R, W, X, U, G, A, D, PBMT))
            return -1;
    }

    return 0;
}

int iommu_ref_iotlb_invalid_vma_all(void)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(-1, -1, -1);
}

int iommu_ref_iotlb_invalid_vma_addr(unsigned long addr)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(addr, -1, -1);
}

int iommu_ref_iotlb_invalid_vma_pscid(unsigned int pscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(-1, -1, pscid);
}

int iommu_ref_iotlb_invalid_vma_gscid(unsigned int gscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(-1, gscid, -1);
}

int iommu_ref_iotlb_invalid_vma_addr_pscid(unsigned long addr, unsigned int pscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(addr, -1, pscid);
}

int iommu_ref_iotlb_invalid_vma_addr_gscid(unsigned long addr, unsigned int gscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(addr, gscid, -1);
}

int iommu_ref_iotlb_invalid_vma_gscid_pscid(unsigned int gscid, unsigned int pscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(-1, gscid, pscid);
}

int iommu_ref_iotlb_invalid_vma_addr_gscid_pscid(unsigned long addr, unsigned int gscid, unsigned int pscid)
{
    if (!iommu_ref_cmdq->ops.inval_vma)
        return -1;

    return iommu_ref_cmdq->ops.inval_vma(addr, gscid, pscid);
}

int iommu_ref_iotlb_invalid_gvma_all(void)
{
    if (!iommu_ref_cmdq->ops.inval_gvma)
        return -1;

    return iommu_ref_cmdq->ops.inval_gvma(-1, -1);
}

int iommu_ref_iotlb_invalid_gvma_gscid(unsigned int gscid)
{
    if (!iommu_ref_cmdq->ops.inval_gvma)
        return -1;

    return iommu_ref_cmdq->ops.inval_gvma(-1, gscid);
}

int iommu_ref_iotlb_invalid_gvma_addr_gscid(uint64_t addr, int gscid)
{
    if (!iommu_ref_cmdq->ops.inval_gvma)
        return -1;

    return iommu_ref_cmdq->ops.inval_gvma(addr, gscid);
}

int iommu_ref_iofence(void)
{
    if (!iommu_ref_cmdq->ops.iofence)
        return -1;

    return iommu_ref_cmdq->ops.iofence();
}

int iommu_ref_iofence_set_av(uint32_t addr, uint32_t data)
{
    if (!iommu_ref_cmdq->ops.iofence_set_av)
        return -1;

    return iommu_ref_cmdq->ops.iofence_set_av(addr, data);
}

int iommu_ref_iodir_ddt_all(void)
{
    if (!iommu_ref_cmdq->ops.inval_ddt)
        return -1;

    return iommu_ref_cmdq->ops.inval_ddt(-1);
}

int iommu_ref_iodir_ddt(uint32_t did)
{
    if (!iommu_ref_cmdq->ops.inval_ddt)
        return -1;

    return iommu_ref_cmdq->ops.inval_ddt(did);
}

int iommu_ref_iodir_pdt(uint32_t did, uint32_t pid)
{
    if (!iommu_ref_cmdq->ops.inval_pdt)
        return -1;

    return iommu_ref_cmdq->ops.inval_pdt(did, pid);
}

int iommu_ref_cmdq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data)
{
    if (!iommu_ref_cmdq->ops.set_msi)
        return -1;

    return iommu_ref_cmdq->ops.set_msi(idx, msi_addr, msi_data);
}

int iommu_ref_cmdq_process_intr(void)
{
    if (!iommu_ref_cmdq->ops.process_intr)
        return -1;

    iommu_ref_cmdq->ops.process_intr();

    return 0;
}

int iommu_ref_fltq_process_intr(void)
{
    if (!iommu_ref_fltq->ops.process_intr)
        return -1;

    iommu_ref_fltq->ops.process_intr();

    return 0;
}

int iommu_ref_fltq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data)
{
    if (!iommu_ref_fltq->ops.set_msi)
        return -1;

    return iommu_ref_fltq->ops.set_msi(idx, msi_addr, msi_data);
}

int iommu_ref_fltq_init(uint64_t base, int log2size,
                        void (*write_l)(uint64_t addr, uint32_t val),
		        uint32_t (*read_l)(uint64_t addr),
		        void (*write_q)(uint64_t addr, uint64_t val),
		        uint64_t (*read_q)(uint64_t addr))
{
    return iommu_fltq_init(base, log2size, write_l, read_l, write_q, read_q);
}

int iommu_ref_cmdq_init(uint64_t base, int log2size,
                        void (*write_l)(uint64_t addr, uint32_t val),
                        uint32_t (*read_l)(uint64_t addr),
                        void (*write_q)(uint64_t addr, uint64_t val),
                        uint64_t (*read_q)(uint64_t addr))
{
    return iommu_cmdq_init(base, log2size, write_l, read_l, write_q, read_q);
}

int iommu_check_dbg_response(unsigned long val, unsigned long pa,
                             int level, int pbmt, int fault)
{
    unsigned long ppn;

    my_print("%s -- tr_response:0x%lx, expected -- ppn:0x%lx level:%d pbmt:%d fault:%d\n",
              __FUNCTION__, val, pa >> 12, level, pbmt, fault);

    if (fault != get_field(val, TR_RES_FAULT)) {
        my_print("%s -- fault check failed\n", __FUNCTION__);
        return -1;
    }

    if (pbmt != get_field(val, TR_RES_PBMT)) {
        my_print("%s -- pbmt check failed\n", __FUNCTION__);
        return -1;
    }

    if (!level && get_field(val, TR_RES_S)) {
        my_print("%s -- S check failed\n", __FUNCTION__);
        return -1;
    }

    if (level && !get_field(val, TR_RES_S)) {
        my_print("%s -- S check failed\n", __FUNCTION__);
        return -1;
    }

    ppn = get_field(val, TR_RES_PPN);
    if (level) {
        int pos = 0;
	int pgsz = 2;
        unsigned long page_size;
        while (ppn & (1UL << pos)) {
            pgsz = pgsz << 1;
            pos++;
        }
        page_size = 4096 * pgsz;
        if (page_size != (4096 << (level * 9))) {
            my_print("%s -- PPN check failed, page_size:%d expected:%d\n",
                      __FUNCTION__, page_size, (4096 << (level * 9)));
            return -1;
        }
    } else {
        if (get_field(val, TR_RES_PPN) != (pa >> 12)) {
            my_print("%s -- PPN check failed, PPN:0x%lx expected:0x%lx\n",
                      __FUNCTION__, get_field(val, TR_RES_PPN), pa >> 12);
            return -1;
        }
    }

    my_print("check pass!!\n");
    return 0;
}

uint64_t iommu_ref_dbg(uint64_t base, uint64_t iova,
                       uint32_t did, uint32_t pid,
                       int pv, int exe, int nw, int priv,
                       void (*write_q)(uint64_t addr, uint64_t val),
                       uint64_t (*read_q)(uint64_t addr))
{
    unsigned long tr_req_iova = 0;
    unsigned long tr_req_ctl = 0;

    my_print("start %s -- iova:0x%lx did:0x%x pid:0x%x pv:%d exe:%d nw:%d priv:%d\n",
              __FUNCTION__, iova, did, pid, pv, exe, nw, priv);

    tr_req_iova = set_field(tr_req_iova, TR_REQ_IOVA_VPN, iova >> 12);

    tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_DID, did);
    if (pv == 1) {
        tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_PV, pv);
        tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_PID, pid);
    }
    tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_EXE, exe);
    tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_NW, nw);
    tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_PRIV, priv);
    tr_req_ctl = set_field(tr_req_ctl, TR_REQ_CTL_GO_BUSY, 1);

    my_print("%s -- tr_req_iova:0x%lx tr_req_ctl:0x%lx\n", __FUNCTION__, tr_req_iova, tr_req_ctl);

    while (get_field(read_q(base + TR_REQ_CTRL_OFFSET), TR_REQ_CTL_GO_BUSY)) ;

    write_q(base + TR_REQ_IOVA_OFFSET, tr_req_iova);
    write_q(base + TR_REQ_CTRL_OFFSET, tr_req_ctl);

    while (get_field(read_q(base + TR_REQ_CTRL_OFFSET), TR_REQ_CTL_GO_BUSY)) ;

    return read_q(base + TR_RESPONSE_OFFSET);
}

uint64_t
enable_iommu(
    uint8_t iommu_mode) {
    ddtp_t ddtp;
    uint32_t i;
    uint64_t zero = 0;

    // Allocate a page for DDT root page
    do {
        ddtp.raw = read_register(DDTP_OFFSET, 8);
    } while ( ddtp.busy == 1 );

    ddtp.ppn = get_free_ppn(1);
    // Clear the page
    for ( i = 0; i < 512; i++ )
        write_memory_test((char *)&zero, (ddtp.ppn * PAGESIZE) | (i * 8), 8);

    ddtp.iommu_mode = iommu_mode;
    write_register(DDTP_OFFSET, 8, ddtp.raw);
    do {
        ddtp.raw = read_register(DDTP_OFFSET, 8);
    } while ( ddtp.busy == 1 );
    return ddtp.raw;
}

uint64_t
add_device(uint32_t device_id, uint32_t process_id,
           uint32_t gscid, uint32_t pscid,
           uint8_t en_ats, uint8_t en_pri,
           uint8_t t2gpa, uint8_t dtf, uint8_t prpr,
           uint8_t gade, uint8_t sade, uint8_t dpe,
           uint8_t sbe, uint8_t sxl, uint8_t iohgatp_mode,
           uint8_t iosatp_mode, uint8_t pdt_mode,
           uint8_t msiptp_mode, uint8_t msiptp_pages,
           uint64_t msi_addr_mask, uint64_t msi_addr_pattern,
           uint8_t ENS, uint32_t SUM, uint64_t *pc_addr)
{
    device_context_t DC;
    char *zero;

    zero = (char *)my_alloc(16384);
    //memset(&zero, 0, 16384);
    memset(&DC, 0, sizeof(DC));

    DC.tc.V      = 1;
    DC.tc.EN_ATS = en_ats;
    DC.tc.EN_PRI = en_pri;
    DC.tc.T2GPA  = t2gpa;
    DC.tc.DTF    = dtf;
    DC.tc.PRPR   = prpr;
    DC.tc.GADE   = gade;
    DC.tc.SADE   = sade;
    DC.tc.DPE    = dpe;
    DC.tc.SBE    = sbe;
    DC.tc.SXL    = sxl;
    if ( iohgatp_mode != IOHGATP_Bare ) {
        DC.iohgatp.GSCID = gscid;
        DC.iohgatp.PPN = get_free_ppn(4);
        write_memory_test(zero, DC.iohgatp.PPN * PAGESIZE, 16384);
    }
    DC.iohgatp.MODE = iohgatp_mode;
    if ( iosatp_mode != IOSATP_Bare ) {
        DC.tc.PDTV = 0;
        DC.fsc.iosatp.MODE = iosatp_mode;
        if ( DC.iohgatp.MODE != IOHGATP_Bare ) {
            gpte_t gpte;
            DC.fsc.iosatp.PPN = get_free_gppn(1, DC.iohgatp.raw);
            my_print("iosatp.PPN:0x%lx\n", DC.fsc.iosatp.PPN);
            gpte.raw = 0;
            gpte.V = 1;
            gpte.R = 1;
            gpte.W = 1;
            gpte.X = 0;
            gpte.U = 1;
            gpte.G = 0;
            gpte.A = 1;
            gpte.D = 1;
            gpte.PBMT = PMA;
            gpte.PPN = get_free_ppn(1);
            write_memory_test(zero, gpte.PPN * PAGESIZE, 4096);
            add_g_stage_pte(DC.iohgatp, (PAGESIZE * DC.fsc.iosatp.PPN), gpte, 0);
        } else {
            DC.fsc.iosatp.PPN = get_free_ppn(1);
            write_memory_test(zero, DC.fsc.iosatp.PPN * PAGESIZE, 4096);
        }
    }
    if ( pdt_mode != PDTP_Bare ) {
        process_context_t PC = { 0 };
        DC.tc.PDTV = 1;
        DC.fsc.pdtp.MODE = pdt_mode;
        if ( DC.iohgatp.MODE != IOHGATP_Bare ) {
            gpte_t gpte;
            DC.fsc.pdtp.PPN = get_free_gppn(1, DC.iohgatp.raw);
            gpte.raw = 0;
            gpte.V = 1;
            gpte.R = 1;
            gpte.W = 1;
            gpte.X = 0;
            gpte.U = 1;
            gpte.G = 0;
            gpte.A = 1;
            gpte.D = 1;
            gpte.PBMT = PMA;
            gpte.PPN = get_free_ppn(1);
            write_memory_test(zero, gpte.PPN * PAGESIZE, 4096);
            add_g_stage_pte(DC.iohgatp, (PAGESIZE * DC.fsc.pdtp.PPN), gpte, 0);
        } else {
            DC.fsc.pdtp.PPN = get_free_ppn(1);
            write_memory_test(zero, DC.fsc.pdtp.PPN * PAGESIZE, 4096);
        }

        PC.fsc.iosatp.MODE = iosatp_mode;
        if (iohgatp_mode != IOHGATP_Bare) {
            gpte_t gpte;
            PC.fsc.iosatp.PPN = get_free_gppn(1, DC.iohgatp.raw);
            gpte.raw = 0;
            gpte.V = 1;
            gpte.R = 1;
            gpte.W = 1;
            gpte.X = 0;
            gpte.U = 1;
            gpte.G = 0;
            gpte.A = 1;
            gpte.D = 1;
            gpte.PBMT = PMA;
            gpte.PPN = get_free_ppn(1);
            add_g_stage_pte(DC.iohgatp, PC.fsc.iosatp.PPN * PAGESIZE, gpte, 0);
        }
        else
            PC.fsc.iosatp.PPN = get_free_ppn(1);
        PC.ta.V = 1;
        PC.ta.PSCID = pscid;
        PC.ta.ENS = ENS;
        PC.ta.SUM = SUM;
        *pc_addr = add_process_context(&DC, &PC, process_id);
    }
    DC.msiptp.MODE = msiptp_mode;
    if ( msiptp_mode != MSIPTP_Off ) {
       DC.msiptp.PPN = get_free_ppn(msiptp_pages);
       write_memory_test(zero, DC.msiptp.PPN * PAGESIZE, 4096);
       DC.msi_addr_mask.mask = msi_addr_mask;
       DC.msi_addr_pattern.pattern = msi_addr_pattern;
    }

    my_print("%s -- DC.fsc.iosatp.PPN:0x%lx DC.iohgatp:0x%lx\n", __FUNCTION__, DC.fsc.iosatp.PPN, DC.iohgatp);
    return add_dev_context(&DC, device_id);
}

int libiommu_ref_init(struct memory_ops *ops)
{
    capabilities_t cap = {0};
    fctl_t fctl = {0};
    uint64_t sv57_bare_sz, sv48_bare_sz, sv39_bare_sz, sv32_bare_sz;

    cap.version = 0x10;
    cap.Sv39 = cap.Sv48 = cap.Sv57 = cap.Sv39x4 = cap.Sv48x4 = cap.Sv57x4 = 1;
    cap.amo_hwad = cap.ats = cap.t2gpa = cap.hpm = cap.msi_flat = cap.msi_mrif = cap.amo_mrif = 1;
    cap.dbg = 1;
    cap.pas = 50;
    cap.pd20 = cap.pd17 = cap.pd8 = 1;
    sv57_bare_sz = sv48_bare_sz = sv39_bare_sz = 0x40000000;
    sv32_bare_sz = 0x200000;
    if ( reset_iommu(8, 40, 0xff, 3, Off, DDT_3LVL, 0xFFFFFF, 0, 0,
                     (FILL_IOATC_ATS_T2GPA | FILL_IOATC_ATS_ALWAYS),
                     cap, fctl, sv57_bare_sz, sv48_bare_sz, sv39_bare_sz,
                     sv32_bare_sz) < 0 )
        return -1;

    set_memory_ops(ops);

    return 0;
}
