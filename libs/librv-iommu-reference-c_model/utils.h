#ifndef _UTILS_H__
#define _UTILS_H__

#include "wrapper.h"
#include "iommu_req_rsp.h"
#include "iommu_ats.h"

uint64_t _pext_u64(uint64_t val, uint64_t ext);
void my_print(const char *fmt, ...);
void set_memory_ops(struct memory_ops *ops);
uint8_t
write_memory_test(char *data, uint64_t addr, uint32_t size);
uint8_t
write_memory(char *data, uint64_t addr,
	     uint32_t size, uint32_t rcid,
	     uint32_t mcid, uint32_t pma);
uint8_t
read_memory_for_AMO(uint64_t addr, uint8_t size,
		    char *data, uint32_t rcid,
		    uint32_t mcid, uint32_t pma);
uint8_t
read_memory_test(uint64_t addr, uint8_t size, char *data);
uint8_t
read_memory(uint64_t addr, uint8_t size,
	    char *data, uint32_t rcid,
	    uint32_t mcid, uint32_t pma);
uint64_t get_free_ppn(uint64_t num_ppn);
uint64_t get_free_gppn(uint64_t num_gppn, uint64_t _iohgatp);
void get_attribs_from_req(hb_to_iommu_req_t *req, uint8_t *read, uint8_t *write, uint8_t *exec, uint8_t *priv);
void send_msg_iommu_to_hb(ats_msg_t *msg);
void iommu_to_hb_do_global_observability_sync(uint8_t PR, uint8_t PW);

#endif
