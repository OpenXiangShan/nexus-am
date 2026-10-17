#ifndef __LIBIOMMU_WRAPPER_H__
#define __LIBIOMMU_WRAPPER_H__

#include <stdint.h>

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

struct memory_ops {
	int      (*vprint)(const char *fmt, va_list ap);
	uint8_t  (*read)(char *addr, char *data, uint32_t size);
	uint8_t  (*write)(char *addr, char *data, uint32_t size);
	uint64_t (*get_free_ppn)(uint64_t num_ppn);
	uint64_t (*mm_alloc)(int size);
	void     (*mm_free)(uint64_t addr, int size);
};
uint64_t build_s1_s2_pte_and_get_leaf(void *dc_addr, unsigned long iova,
                                      unsigned long pa, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT,
                                      unsigned long *s1_pte, unsigned long *s2_pte);

uint64_t build_vs_stage_pte(void *dc_addr, unsigned long iova,
                            unsigned long gpa, int8_t add_level,
                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                            int8_t G, int8_t A, int8_t D, int8_t PBMT);

uint64_t build_basic_msi_pte(void *dc_addr, unsigned long msi_gpa, unsigned long msi_hpa);
uint64_t build_mrif_pte(void *dc_addr, unsigned long msi_gpa, unsigned long *mrif_addr,
			unsigned long notice_msi_addr, unsigned long notice_msi_data);

uint64_t build_s1_s2_pte(void *dc_addr, unsigned long iova,
                         unsigned long pa, int8_t add_level,
                         int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                         int8_t G, int8_t A, int8_t D, int8_t PBMT);

uint64_t build_s1_s2_pte_pc(void *pc_addr, void *dc_addr, unsigned long iova,
                            unsigned long pa, int8_t add_level,
                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                            int8_t G, int8_t A, int8_t D, int8_t PBMT);

uint64_t build_s1_only_pte_pc(void *pc_addr, unsigned long va,
                              unsigned long pa, int8_t add_level,
                              int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                              int8_t G, int8_t A, int8_t D, int8_t PBMT);
uint64_t build_s1_only_pte(void *dc_addr, unsigned long va,
                           unsigned long pa, int8_t add_level,
                           int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                           int8_t G, int8_t A, int8_t D, int8_t PBMT);
uint64_t build_s2_only_pte(void *dc_addr, unsigned long va,
                           unsigned long pa, int8_t add_level,
                           int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                           int8_t G, int8_t A, int8_t D, int8_t PBMT);
uint64_t build_s1_only_pte_napot(void *dc_addr, unsigned long va,
                                 unsigned long pa, int8_t add_level,
                                 int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                 int8_t G, int8_t A, int8_t D, int8_t PBMT, int page_size);

int8_t iommu_ref_s1_only_page_mapping_napot(void *dc_addr, unsigned long va,
                                            unsigned long pa, int size, int page_size,
                                            int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                            int8_t G, int8_t A, int8_t D, int8_t PBMT);
int8_t iommu_ref_s1_only_page_mapping(void *dc_addr, unsigned long va,
                                      unsigned long pa, int size, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT);
int8_t iommu_ref_s2_only_page_mapping(void *dc_addr, unsigned long gpa,
                                      unsigned long pa, int size, int8_t add_level,
                                      int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                      int8_t G, int8_t A, int8_t D, int8_t PBMT);
int8_t iommu_ref_s1_s2_page_mapping(void *dc_addr, unsigned long va,
                                    unsigned long pa, int size, int8_t add_level,
                                    int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                    int8_t G, int8_t A, int8_t D, int8_t PBMT);
int8_t iommu_ref_s1_s2_page_mapping_and_get_leaf(void *dc_addr, unsigned long va,
                                                 unsigned long pa, int size, int8_t add_level,
                                                 int8_t V, int8_t R, int8_t W, int8_t X, int8_t U,
                                                 int8_t G, int8_t A, int8_t D, int8_t PBMT,
                                                 unsigned long *s1_pte, unsigned long *s2_pte);
uint64_t iommu_ref_dbg(uint64_t base, uint64_t iova,
                       uint32_t did, uint32_t pid,
                       int pv, int exe, int nw, int priv,
                       void (*write_q)(uint64_t addr, uint64_t val),
                       uint64_t (*read_q)(uint64_t addr));
int iommu_check_dbg_response(unsigned long val, unsigned long pa,
                             int level, int pbmt, int fault);

int libiommu_ref_init(struct memory_ops *ops);
uint64_t enable_iommu(uint8_t iommu_mode);
uint64_t add_device(uint32_t device_id, uint32_t process_id,
                    uint32_t gscid, uint32_t pscid,
                    uint8_t en_ats, uint8_t en_pri, uint8_t t2gpa,
                    uint8_t dtf, uint8_t prpr,
                    uint8_t gade, uint8_t sade, uint8_t dpe, uint8_t sbe, uint8_t sxl,
                    uint8_t iohgatp_mode, uint8_t iosatp_mode, uint8_t pdt_mode,
                    uint8_t msiptp_mode, uint8_t msiptp_pages, uint64_t msi_addr_mask,
                    uint64_t msi_addr_pattern,
                    uint8_t ENS, uint32_t SUM, uint64_t *pc_addr);
uint64_t my_alloc(int size);
void my_free(uint64_t addr, int size);

int iommu_ref_iodir_pdt(uint32_t did, uint32_t pid);
int iommu_ref_iodir_pdt(uint32_t did, uint32_t pid);
int iommu_ref_iodir_ddt(uint32_t did);
int iommu_ref_iodir_ddt_all(void);
int iommu_ref_iofence_set_av(uint32_t addr, uint32_t data);
int iommu_ref_iofence(void);
int iommu_ref_iotlb_invalid_gvma_addr_gscid(uint64_t addr, int gscid);
int iommu_ref_iotlb_invalid_gvma_gscid(unsigned int gscid);
int iommu_ref_iotlb_invalid_gvma_all(void);
int iommu_ref_iotlb_invalid_vma_addr_gscid_pscid(unsigned long addr, unsigned int gscid, unsigned int pscid);
int iommu_ref_iotlb_invalid_vma_gscid_pscid(unsigned int gscid, unsigned int pscid);
int iommu_ref_iotlb_invalid_vma_addr_gscid(unsigned long addr, unsigned int gscid);
int iommu_ref_iotlb_invalid_vma_addr_pscid(unsigned long addr, unsigned int pscid);
int iommu_ref_iotlb_invalid_vma_gscid(unsigned int gscid);
int iommu_ref_iotlb_invalid_vma_pscid(unsigned int pscid);
int iommu_ref_iotlb_invalid_vma_addr(unsigned long addr);
int iommu_ref_iotlb_invalid_vma_all(void);
int iommu_ref_cmdq_process_intr(void);
int iommu_ref_cmdq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data);
int iommu_ref_fltq_process_intr(void);
int iommu_ref_fltq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data);
int iommu_ref_cmdq_init(uint64_t base, int log2size,
                        void (*write_l)(uint64_t addr, uint32_t val),
                        uint32_t (*read_l)(uint64_t addr),
                        void (*write_q)(uint64_t addr, uint64_t val),
                        uint64_t (*read_q)(uint64_t addr));
int iommu_ref_fltq_init(uint64_t base, int log2size,
                        void (*write_l)(uint64_t addr, uint32_t val),
		        uint32_t (*read_l)(uint64_t addr),
		        void (*write_q)(uint64_t addr, uint64_t val),
		        uint64_t (*read_q)(uint64_t addr));

#endif
