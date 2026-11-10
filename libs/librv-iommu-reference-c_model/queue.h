#ifndef __IOMMU_REF_QUEUE_H__
#define __IOMMU_REF_QUEUE_H__

#define CQ_LOG2SZ_1     (5  )
// Mask for cqb.PPN (cqb[53:10])
#define CQB_PPN_MASK    (0x3FFFFFFFFFFC00ULL)
// Mask for CQ PPN (cqb[55:12])
#define CQ_PPN_MASK        (0xFFFFFFFFFFF000ULL)

#define FQ_LOG2SZ_1     (5 )
// Mask for cqb.PPN (fqb[53:10])
#define FQB_PPN_MASK    (0x3FFFFFFFFFFC00ULL)
// Mask for CQ PPN (cqb[55:12])
#define FQ_PPN_MASK     (0xFFFFFFFFFFF000ULL)

// cqcsr masks
#define CQCSR_CQEN          (1UL << 0)
#define CQCSR_CIE           (1UL << 1)
#define CQCSR_CQMF          (1UL << 8)
#define CQCSR_CMD_TO        (1UL << 9)
#define CQCSR_CMD_ILL       (1UL << 10)
#define CQCSR_FENCE_W_IP    (1UL << 11)
#define CQCSR_CQON          (1UL << 16)
#define CQCSR_BUSY          (1UL << 17)

// fqcsr masks
#define FQCSR_CQEN          (1UL << 0)
#define FQCSR_CIE           (1UL << 1)
#define FQCSR_CQMF          (1UL << 8)
#define FQCSR_CMD_TO        (1UL << 9)
#define FQCSR_CMD_ILL       (1UL << 10)
#define FQCSR_FENCE_W_IP    (1UL << 11)
#define FQCSR_CQON          (1UL << 16)
#define FQCSR_BUSY          (1UL << 17)

// iofence
#define IOMMU_CMD_IOFENCE_OPCODE         (2UL)
#define IOMMU_CMD_IOFENCE_FUNC_IOFENCE_C (0UL << 7)
#define IOMMU_CMD_IOFENCE_AV             (1UL << 10)
#define IOMMU_CMD_IOFENCE_WSI            (1UL << 11)
#define IOMMU_CMD_IOFENCE_PR             (1UL << 12)
#define IOMMU_CMD_IOFENCE_PW             (1UL << 13)
#define IOMMU_CMD_IOFENCE_DATA           (0xffffffffUL << 32)

// iodir invalidation
#define IOMMU_CMD_IODIR_OPCODE 	  (3UL)
#define IOMMU_CMD_IODIR_FUNC3_DDT (0UL << 7)
#define IOMMU_CMD_IODIR_FUNC3_PDT (1UL << 7)
#define IOMMU_CMD_IODIR_PID_SHIFT 12
#define IOMMU_CMD_IODIR_PID_MASK  (0xFFFFF000UL)
#define IOMMU_CMD_IODIR_DV        (1UL << 33)
#define IOMMU_CMD_IODIR_DID_SHIFT 40
#define IOMMU_CMD_IODIR_DID_MASK  (0xffffff0000000000)

// iotlb invalidatoin
#define IOMMU_CMD_IOTINVAL_OPCODE      (1UL)
#define IOMMU_CMD_IOTINVAL_FUNC3_VMA   (0UL << 7)
#define IOMMU_CMD_IOTINVAL_FUNC3_GVMA  (1UL << 7)
#define IOMMU_CMD_IOTINVAL_AV          (1UL << 10)
#define IOMMU_CMD_IOTINVAL_PSCID_SHIFT 12
#define IOMMU_CMD_IOTINVAL_PSCID_MASK  (0xFFFFF000UL)
#define IOMMU_CMD_IOTINVAL_PSCV        (1UL << 32)
#define IOMMU_CMD_IOTINVAL_GV          (1UL << 33)
#define IOMMU_CMD_IOTINVAL_GSCID_SHIFT 44
#define IOMMU_CMD_IOTINVAL_GSCID_MASK  (0xFFFF00000000000UL)
#define IOMMU_CMD_IOTINVAL_ADDR_SHIFT  10
#define IOMMU_CMD_IOTINVAL_ADDR_MASK   (0xFFFFFC00UL)

struct iommu_cmd_ops {
	int (*inval_ddt)(unsigned int devid);
	int (*inval_pdt)(unsigned int devid, unsigned int process_id);
	int (*inval_vma)(unsigned long addr, int gscid, int pscid);
	int (*inval_gvma)(unsigned long addr, int gscid);
	int (*iofence)(void);
	int (*iofence_set_av)(unsigned long addr, unsigned int data);
	int (*set_msi)(int idx, uint64_t msi_addr, uint64_t msi_data);
	void (*process_intr)(void);
};

struct iommu_fltq_ops {
	int (*fetch)(int *idx);
	void (*release)(int idx);
	void (*report)(int idx);
	int (*set_msi)(int idx, uint64_t msi_addr, uint64_t msi_data);
	void (*process_intr)(void);
};

struct command {
	unsigned long low_64;
	unsigned long high_64;
};

struct iommu_fltq_queue {
	int init;
	unsigned long base;
	int irq;
	unsigned int fqb;
	unsigned int fqcsr;
	int count;
	struct iommu_fltq_ops ops;
};

struct iommu_cmd_queue {
	int init;
	unsigned long base;
	int irq;
	unsigned int cqb;
	unsigned int cqcsr;
	int count;
	struct iommu_cmd_ops ops;
};

struct iommu_fq_content {
	union {
		struct {
			uint32_t cause : 12;
			uint32_t pid   : 20;
			uint32_t pv    :  1;
			uint32_t priv  :  1;
			uint32_t ttyp  :  6;
			uint32_t did   : 24;
		};
		unsigned long hdr;
	};
	unsigned long reserved;
	unsigned long iotval;
	unsigned long iotval2;
};

struct iommu_cmd_queue *get_iommu_ref_cmdq(void);
struct iommu_fltq_queue *get_iommu_ref_fltq(void);
int iommu_cmdq_init(uint64_t base, int log2size,
                    void (*write_l)(uint64_t addr, uint32_t val),
                    uint32_t (*read_l)(uint64_t addr),
                    void (*write_q)(uint64_t addr, uint64_t val),
                    uint64_t (*read_q)(uint64_t addr));
int iommu_fltq_init(uint64_t base, int log2size,
		    void (*write_l)(uint64_t addr, uint32_t val),
		    uint32_t (*read_l)(uint64_t addr),
		    void (*write_q)(uint64_t addr, uint64_t val),
		    uint64_t (*read_q)(uint64_t addr));

#define iommu_ref_cmdq get_iommu_ref_cmdq()
#define iommu_ref_fltq get_iommu_ref_fltq()

#endif
