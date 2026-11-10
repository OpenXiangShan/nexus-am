#include <string.h>
#include <stdint.h>
#include "queue.h"
#include "utils.h"
#include "barrier.h"
#include "iommu_registers.h"

/* RISC-V IOMMU PPN <> PHYS address conversions, PHYS <=> PPN[53:10] */
#define phys_to_ppn(pa)  (((pa) >> 2) & (((1ULL << 44) - 1) << 10))
#define ppn_to_phys(pn)  (((pn) << 2) & (((1ULL << 44) - 1) << 12))

typedef void (*writel_t)(uint64_t addr, uint32_t val);
typedef uint32_t (*readl_t)(uint64_t addr);
typedef void (*writeq_t)(uint64_t addr, uint64_t val);
typedef uint64_t (*readq_t)(uint64_t addr);

static writel_t __writel;
static readl_t __readl;
static writeq_t __writeq;
static readq_t __readq;
static uint64_t base_addr;
static struct iommu_cmd_queue command_queue = { 0 };
static struct iommu_fltq_queue fltq_queue = { 0 };

static int iommu_command_post(uint64_t base, struct command *cmd, int sync)
{
	unsigned int head, tail, next;
	struct iommu_cmd_queue *cmdq = &command_queue;
	unsigned int max = cmdq->count - 1;
	struct command *cmd_q_buf = (struct command *)cmdq->base;

	head = __readl(base + CQH_OFFSET);
	tail = __readl(base + CQT_OFFSET);

	if (tail == max)
		next = 0;
	else
		next = tail + 1;

	while (next == head) {
		head = __readl(base + CQH_OFFSET);
	}

	cmd_q_buf[tail] = *cmd;
	my_print("command sync-- cmd_q_buf->low:0x%x *cmd_q_buf(low):0x%x cmd_q_buf->high:0x%x *cmd_q_buf(high):0x%x\n",
			(unsigned long)&cmd_q_buf[tail].low_64, cmd_q_buf[tail].low_64, (unsigned long)&cmd_q_buf[tail].high_64, cmd_q_buf[tail].high_64);
	wmb();
	my_print("cmd: write %d to 0x%lx\n", next, base + CQT_OFFSET);
	__writel(base + CQT_OFFSET, next);

	if (sync) {
		head = __readl(base + CQH_OFFSET);
		while (head != tail) //we only have one thread, so we only need to judge head==tail
			tail = __readl(base + CQT_OFFSET);
	}

	return 0;
}

static int iommu_command_post_async(uint64_t base, struct command *cmd)
{
	return iommu_command_post(base, cmd, 0);
}

static int iommu_command_post_sync(uint64_t base, struct command *cmd)
{
	return iommu_command_post(base, cmd, 1);
}

static void iommu_generate_inval_ddt_cmd(struct command *cmd, unsigned int did)
{
	cmd->low_64 = IOMMU_CMD_IODIR_OPCODE |
		      IOMMU_CMD_IODIR_FUNC3_DDT;
	if (did != -1)
		cmd->low_64 |=
			((((unsigned long)did) << IOMMU_CMD_IODIR_DID_SHIFT) & IOMMU_CMD_IODIR_DID_MASK) |
			IOMMU_CMD_IODIR_DV;

	cmd->high_64 = 0;
}

static void iommu_generate_inval_pdt_cmd(struct command *cmd, unsigned int did, unsigned int pid)
{
	cmd->low_64 = IOMMU_CMD_IODIR_OPCODE |
		      IOMMU_CMD_IODIR_FUNC3_PDT |
		      ((pid << IOMMU_CMD_IODIR_PID_SHIFT) & IOMMU_CMD_IODIR_PID_MASK) |
		      IOMMU_CMD_IODIR_DV |
		      ((((unsigned long)did) << IOMMU_CMD_IODIR_DID_SHIFT) & IOMMU_CMD_IODIR_DID_MASK);
	cmd->high_64 = 0;
}

static void iommu_generate_iofence_cmd(struct command *cmd)
{
	cmd->low_64 = IOMMU_CMD_IOFENCE_OPCODE |
		      IOMMU_CMD_IOFENCE_FUNC_IOFENCE_C;
	cmd->high_64 = 0;
}

static void iommu_generate_iofence_cmd_set_av(struct command *cmd,
					      unsigned long addr,
					      unsigned int data)
{
	cmd->low_64 = IOMMU_CMD_IOFENCE_OPCODE |
		      IOMMU_CMD_IOFENCE_FUNC_IOFENCE_C |
		      ((((unsigned long)data) << 32) & IOMMU_CMD_IOFENCE_DATA) |
		      IOMMU_CMD_IOFENCE_AV;
	cmd->high_64 = addr >> 2;
}

static void iommu_generate_iotinval_vma(struct command *cmd, unsigned long addr,
					int gscid, int pscid)
{
	cmd->low_64 = IOMMU_CMD_IOTINVAL_OPCODE |
		      IOMMU_CMD_IOTINVAL_FUNC3_VMA;
	cmd->high_64 = 0;

	if (gscid != -1)
		cmd->low_64 |= IOMMU_CMD_IOTINVAL_GV |
			       ((((unsigned long)gscid) << IOMMU_CMD_IOTINVAL_GSCID_SHIFT)
						& IOMMU_CMD_IOTINVAL_GSCID_MASK);

	if (pscid != -1)
		cmd->low_64 |= IOMMU_CMD_IOTINVAL_PSCV |
			       ((((unsigned long)pscid) << IOMMU_CMD_IOTINVAL_PSCID_SHIFT)
						& IOMMU_CMD_IOTINVAL_PSCID_MASK);

	if (addr != -1) {
		cmd->low_64 |= IOMMU_CMD_IOTINVAL_AV;
		cmd->high_64 |= phys_to_ppn(addr);
	}
}

static void iommu_generate_iotinval_gvma(struct command *cmd,
				         unsigned long addr,
					 int gscid)
{
	cmd->low_64 = IOMMU_CMD_IOTINVAL_OPCODE |
		      IOMMU_CMD_IOTINVAL_FUNC3_GVMA;
	cmd->high_64 = 0;

	if (gscid != -1)
		cmd->low_64 |= IOMMU_CMD_IOTINVAL_GV |
			       ((((unsigned long)gscid) << IOMMU_CMD_IOTINVAL_GSCID_SHIFT)
						& IOMMU_CMD_IOTINVAL_GSCID_MASK);

	if (addr != -1) {
		cmd->low_64 |= IOMMU_CMD_IOTINVAL_AV;
		cmd->high_64 |= phys_to_ppn(addr);
	}
}

static int iommu_cmd_iofence(void)
{
	struct command cmd;

	iommu_generate_iofence_cmd(&cmd);
	return iommu_command_post_sync(base_addr, &cmd);
}

static int iommu_inval_ddt(unsigned int devid)
{
	struct command cmd;

	iommu_generate_inval_ddt_cmd(&cmd, devid);

	return iommu_command_post_async(base_addr, &cmd); //7. write cmd to cmdq buffer; 8. write cqt
}

static int iommu_inval_pdt(unsigned int devid, unsigned int pasid)
{
	struct command cmd;

	iommu_generate_inval_pdt_cmd(&cmd, devid, pasid);

	return iommu_command_post_async(base_addr, &cmd); //7. write cmd to cmdq buffer; 8. write cqt
}

static int iommu_iotinval_vma(unsigned long addr, int gscid, int pscid)
{
	struct command cmd;

	iommu_generate_iotinval_vma(&cmd, addr, gscid, pscid);

	return iommu_command_post_async(base_addr, &cmd);
}

static int iommu_iotinval_gvma(unsigned long addr, int gscid)
{
	struct command cmd;

	iommu_generate_iotinval_gvma(&cmd, addr, gscid);

	return iommu_command_post_async(base_addr, &cmd);
}

static int iommu_cmd_iofence_set_av(unsigned long addr,
				    unsigned int data)
{
	struct command cmd;

	iommu_generate_iofence_cmd_set_av(&cmd, addr, data);
	return iommu_command_post_sync(base_addr, &cmd);
}

struct iommu_cmd_queue *get_iommu_ref_cmdq(void)
{
	return &command_queue;
}

struct iommu_fltq_queue *get_iommu_ref_fltq(void)
{
	return &fltq_queue;
}

static void iommu_fltq_process_intr(void)
{
	int idx;
	ipsr_t ipsr;
	fqcsr_t ctl;
	struct iommu_fltq_queue *fltq = &fltq_queue;

	ipsr.raw = __readl(base_addr + IPSR_OFFSET);
	if (!ipsr.fip)
		return;

	ctl.raw = __readl(base_addr + FQCSR_OFFSET);
	if (ctl.fqmf && ctl.fqof) {
		my_print("fq error!! fault:%d full:%d\n",
			  ctl.fqmf, ctl.fqof);
		__writel(base_addr + FQCSR_OFFSET, ctl.raw);
	}

	if (!fltq->ops.fetch) {
		my_print("no fetch in fltq ops...\n");
		return;
	}
	if (!fltq->ops.release) {
		my_print("no release in fltq ops...\n");
		return;
	}

	while (fltq->ops.fetch(&idx)) {
		if (fltq->ops.report)
			fltq->ops.report(idx);
		fltq->ops.release(idx);
	}
}

static void iommu_cmdq_process_intr(void)
{
	ipsr_t ipsr;
	cqcsr_t ctl;

	ipsr.raw = __readl(base_addr + IPSR_OFFSET);
	if (!ipsr.cip)
		return;

	ctl.raw = __readl(base_addr + CQCSR_OFFSET);
	if (ctl.cqmf && ctl.cmd_to && ctl.cmd_ill) {
		my_print("cq error!! fault:%d tout:%d err:%d\n",
				ctl.cqmf, ctl.cmd_to, ctl.cmd_ill);
		__writel(base_addr + CQCSR_OFFSET, ctl.raw);
	}

	ipsr.cip = 1;
	__writel(base_addr + IPSR_OFFSET, ipsr.raw);
}

static int __iommu_queue_set_msi(icvec_t icvec, uint32_t msi_offset,
				 uint64_t msi_addr, uint64_t msi_data)
{
	capabilities_t cap;

	cap.raw = __readl(base_addr + CAPABILITIES_OFFSET);
	if ((cap.igs != MSI) && (cap.igs != IGS_BOTH))
		return -1;

	__writeq(base_addr + ICVEC_OFFSET, icvec.raw);

	__writeq(base_addr + msi_offset, msi_addr);
	__writel(base_addr + msi_offset + 8, msi_data);
	__writel(base_addr + msi_offset + 0xC, 0);

	return 0;
}

static int iommu_fltq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data)
{
	uint32_t msi_offset = idx * 16 + MSI_ADDR_0_OFFSET;
	icvec_t icvec;

	icvec.raw = __readq(base_addr + ICVEC_OFFSET);
	icvec.fiv = idx;

	return __iommu_queue_set_msi(icvec, msi_offset, msi_addr, msi_data);
}

static int iommu_cmdq_set_msi(int idx, uint64_t msi_addr, uint64_t msi_data)
{
	uint32_t msi_offset = idx * 16 + MSI_ADDR_0_OFFSET;
	icvec_t icvec;

	icvec.raw = __readq(base_addr + ICVEC_OFFSET);
	icvec.civ = idx;

	return __iommu_queue_set_msi(icvec, msi_offset, msi_addr, msi_data);
}

static int iommu_fltq_fetch(int *idx)
{
	int tail, head;

	tail = __readl(base_addr + FQT_OFFSET);
	head = __readl(base_addr + FQH_OFFSET);

	if (head == tail)
		return 0;

	*idx = head;

	return 1;
}

static void iommu_fltq_release(int idx)
{
	struct iommu_fltq_queue *fltq = &fltq_queue;
	int max = fltq->count - 1;

	if (idx == max)
		idx = 0;
	else
		idx += 1;

	__writel(base_addr + FQH_OFFSET, idx);
}

static void iommu_fltq_report(int idx)
{
	struct iommu_fltq_queue *fltq = &fltq_queue;
	struct iommu_fq_content *base = (struct iommu_fq_content *)fltq->base;
	struct iommu_fq_content *content = &base[idx];

	my_print("fltq report:\n");
	my_print("      cause:0x%x\n", content->cause);
	my_print("      pid:0x%x\n", content->pid);
	my_print("      pv:0x%x\n", content->pv);
	my_print("      priv:0x%x\n", content->priv);
	my_print("      ttyp:0x%x\n", content->ttyp);
	my_print("      did:0x%x\n", content->did);
	my_print("      iotval: 0x%x\n", content->iotval);
	my_print("      iotval2: 0x%x\n", content->iotval2);
}

int iommu_fltq_init(uint64_t base, int log2size,
		    void (*write_l)(uint64_t addr, uint32_t val),
		    uint32_t (*read_l)(uint64_t addr),
		    void (*write_q)(uint64_t addr, uint64_t val),
		    uint64_t (*read_q)(uint64_t addr))
{
	struct iommu_fltq_queue *fltq = &fltq_queue;
	int count = (1U) << log2size;
	unsigned long fqb;
	fctl_t fctl;

	if (fltq->init)
		return 0;

	base_addr = base;

	__writel = write_l;
	__readl = read_l;
	__writeq = write_q;
	__readq = read_q;

	fltq->base = (unsigned long)my_alloc(4096);
	if (!fltq->base)
		return -1;
	memset((char *)fltq->base, 0, 4096);

	if (count > 32) {
		count = 32;
		log2size = 4;
	}
	fltq->count = count;

	fqb = (((fltq->base) >> 2) & FQB_PPN_MASK) | (log2size - 1);
	__writeq(base_addr + FQB_OFFSET, fqb);

	__writel(base_addr + FQCSR_OFFSET, FQCSR_CQEN | FQCSR_CIE);

	while (!(__readl(base_addr + FQCSR_OFFSET) & FQCSR_CQON));

	fltq->ops.fetch = iommu_fltq_fetch;
	fltq->ops.release = iommu_fltq_release;
	fltq->ops.report = iommu_fltq_report;
	fltq->ops.set_msi = iommu_fltq_set_msi;
	fltq->ops.process_intr = iommu_fltq_process_intr;

	fctl.raw = __readl(base_addr + FCTRL_OFFSET);
	fctl.wsi = 0;
	__writel(base_addr + FCTRL_OFFSET, fctl.raw);

	return 0;
}

int iommu_cmdq_init(uint64_t base, int log2size,
		    void (*write_l)(uint64_t addr, uint32_t val),
		    uint32_t (*read_l)(uint64_t addr),
		    void (*write_q)(uint64_t addr, uint64_t val),
		    uint64_t (*read_q)(uint64_t addr))
{
	struct iommu_cmd_queue *cmdq = &command_queue;
	int count = (1U) << log2size;
	unsigned long cqb;
	unsigned int cqh;
	fctl_t fctl;

	if (cmdq->init)
		return 0;

	base_addr = base;

	__writel = write_l;
	__readl = read_l;
	__writeq = write_q;
	__readq = read_q;

	cmdq->base = (unsigned long)my_alloc(4096);
	if (!cmdq->base)
		return -1;
	memset((char *)cmdq->base, 0, 4096);

	if (count > 32) {
		count = 32;
		log2size = 4;
	}
	cmdq->count = count;

	cqb = (((cmdq->base) >> 2) & CQB_PPN_MASK) | (log2size - 1);
	__writeq(base_addr + CQB_OFFSET, cqb); //1. write base address to cqb

	__writel(base_addr + CQCSR_OFFSET, CQCSR_CQEN | CQCSR_CIE); //4. write 0x11(enable and intr enable bit) to cqcsr(0x48)

	__writel(base_addr + CQH_OFFSET, 0);

	cqh = __readl(base_addr + CQH_OFFSET); //2. read cqh(0x20)
	__writel(base_addr + CQT_OFFSET, cqh); //3. write cqh to cqt(0x24)

	while (!(__readl(base_addr + CQCSR_OFFSET) & CQCSR_CQON)); //5. read cqcsr(0x48) until its bit16 is set

	fctl.raw = __readl(base_addr + FCTRL_OFFSET);
	fctl.wsi = 0;
	__writel(base_addr + FCTRL_OFFSET, fctl.raw);

	cmdq->ops.inval_ddt = iommu_inval_ddt;
	cmdq->ops.inval_pdt = iommu_inval_pdt;
	cmdq->ops.inval_vma = iommu_iotinval_vma;
	cmdq->ops.inval_gvma = iommu_iotinval_gvma;
	cmdq->ops.iofence = iommu_cmd_iofence;
	cmdq->ops.iofence_set_av = iommu_cmd_iofence_set_av;
	cmdq->ops.set_msi = iommu_cmdq_set_msi;
	cmdq->ops.process_intr = iommu_cmdq_process_intr;
	cmdq->init = 1;

	return 0;
}
