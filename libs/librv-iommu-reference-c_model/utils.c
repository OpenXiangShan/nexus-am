#include <stdint.h>
#include <string.h>
#include "utils.h"
#include "wrapper.h"
#include "iommu_fault.h"
#include "iommu_translate.h"

static struct memory_ops *mem_ops = 0;
//static char *pool = 0;

static uint64_t *next_free_gpage = 0;

static uint8_t pr_go_requested;
static uint8_t pw_go_requested;
static ats_msg_t exp_msg;
static ats_msg_t rcvd_msg;
static uint8_t exp_msg_received;
static uint8_t message_received;

uint64_t _pext_u64(uint64_t val, uint64_t ext)
{
    uint64_t ret = 0;
    uint64_t rot = 1;

    while (ext) {
        if (ext & 1) {
            if (val & 1) {
                ret |= rot;
            }
            rot <<= 1;
        }
        val >>= 1;
        ext >>= 1;
    }

    return ret;
}

uint64_t
get_free_gppn(uint64_t num_gppn, uint64_t _iohgatp) {
	iohgatp_t iohgatp = (iohgatp_t)_iohgatp;
	uint64_t free_gppn;

	if (!next_free_gpage)
		next_free_gpage = (uint64_t *)my_alloc(65536);
	free_gppn = next_free_gpage[iohgatp.GSCID];

	if ( free_gppn & (num_gppn -1) ) {
		free_gppn = free_gppn + (num_gppn -1);
		free_gppn = free_gppn & ~(num_gppn -1);
	}
	next_free_gpage[iohgatp.GSCID] = free_gppn + num_gppn;
	return free_gppn;
}

uint8_t
read_memory(uint64_t addr, uint8_t size,
	    char *data, uint32_t rcid,
	    uint32_t mcid, uint32_t pma){
	if ( addr == -1 ) return ACCESS_FAULT;
	if ( addr == -1 ) return DATA_CORRUPTION;
	if (!mem_ops || !mem_ops->read)
		return ACCESS_FAULT;
	return mem_ops->read((char *)addr, data, size);
}

uint8_t
read_memory_test(uint64_t addr, uint8_t size, char *data)
{
	return read_memory(addr, size, data, 0, 0, PMA);
}

uint8_t
read_memory_for_AMO(uint64_t addr, uint8_t size,
		    char *data, uint32_t rcid,
		    uint32_t mcid, uint32_t pma) {
	// Same for now
	return read_memory(addr, size, data, rcid, mcid, pma);
}

uint8_t
write_memory(char *data, uint64_t addr,
	     uint32_t size, uint32_t rcid,
	     uint32_t mcid, uint32_t pma) {
	if ( addr == -1 ) return ACCESS_FAULT;
	if ( addr == -1 ) return DATA_CORRUPTION;
	if (!mem_ops || !mem_ops->write)
		return ACCESS_FAULT;
	
	return mem_ops->write((char*)addr, data, size);
}

uint8_t
write_memory_test(char *data, uint64_t addr, uint32_t size)
{
	return write_memory(data, addr, size, 0, 0, PMA);
}

uint64_t get_free_ppn(uint64_t num_ppn)
{
	if (!mem_ops || !mem_ops->get_free_ppn)
		return -1;

	return mem_ops->get_free_ppn(num_ppn);
}

void
iommu_to_hb_do_global_observability_sync(
    uint8_t PR, uint8_t PW){
    pr_go_requested = PR;
    pw_go_requested = PW;
    return;
}

void
send_msg_iommu_to_hb(
    ats_msg_t *msg) {
    if ( exp_msg.MSGCODE != msg->MSGCODE ||
         exp_msg.TAG != msg->TAG ||
         exp_msg.RID != msg->RID ||
         exp_msg.PV  != msg->PV ||
         exp_msg.PID != msg->PID ||
         exp_msg.PRIV != msg->PRIV ||
         exp_msg.EXEC_REQ != msg->EXEC_REQ ||
         exp_msg.DSV != msg->DSV ||
         exp_msg.DSEG != msg->DSEG ||
         exp_msg.PAYLOAD != msg->PAYLOAD )
        exp_msg_received = 0;
    else
        exp_msg_received = 1;
    message_received = 1;
    memcpy(&rcvd_msg, msg, sizeof(ats_msg_t));
    return;
}

void
get_attribs_from_req(
    hb_to_iommu_req_t *req, uint8_t *read, uint8_t *write, uint8_t *exec, uint8_t *priv) {

    *read = ( req->tr.read_writeAMO == READ ) ? 1 : 0;
    *write = ( req->tr.read_writeAMO == WRITE ) ?  1 : 0;

    // The No Write flag, when Set, indicates that the Function is requesting read-only
    // access for this translation.
    // The TA (IOMMU) may ignore the No Write Flag, however, if the TA responds with a
    // translation marked as read-only then the Function must not issue Memory Write
    // transactions using that translation. In this case, the Function may issue another
    // translation request with the No Write flag Clear, which may result in a new
    // translation completion with or without the W (Write) bit Set.
    // Upon receiving a Translation Request with the NW flag Clear, TAs are permitted to
    // mark the associated pages dirty. Functions MUST not issue such Requests
    // unless they have been given explicit write permission.
    // Note ATS Translation requests are read - so read_writeAMO is READ for these requests
    *write = ( (req->tr.at == ADDR_TYPE_PCIE_ATS_TRANSLATION_REQUEST) &&
                 (req->no_write == 0) ) ? 1 : *write;

    // If a Translation Request has a PASID, the Untranslated Address Field is an address
    // within the process address space indicated by the PASID field.
    // If a Translation Request has a PASID with either the Privileged Mode Requested
    // or Execute Requested bit Set, these may be used in constructing the Translation
    // Completion Data Entry.  The PASID Extended Capability indicates whether a Function
    // supports and is enabled to send and receive TLPs with the PASID.
    *exec = ( (*read && req->exec_req &&
                (req->tr.at == ADDR_TYPE_UNTRANSLATED || req->pid_valid)) ) ? 1 : 0;
    *priv = ( req->pid_valid && req->priv_req ) ? S_MODE : U_MODE;
    return;
}

uint64_t my_alloc(int size)
{
	if (!mem_ops || !mem_ops->mm_alloc)
		return -1;

	return mem_ops->mm_alloc(size);
}

void my_free(uint64_t addr, int size)
{
	if (!mem_ops || !mem_ops->mm_free)
		return;

	mem_ops->mm_free(addr, size);
}

void my_print(const char *fmt, ...)
{
	va_list ap;

	if (!mem_ops || !mem_ops->vprint)
		return;

	va_start(ap, fmt);
	mem_ops->vprint(fmt, ap);
	va_end(ap);
}

void set_memory_ops(struct memory_ops *ops)
{
	mem_ops = ops;
}
