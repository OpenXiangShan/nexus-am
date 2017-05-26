#define TLB_LEN 64
#define TLB_MASK 0xFFF

typedef struct {
	PTE pte;
	uint32_t tag : 20;
	bool valid;
} TLB;

TLB tlb[TLB_LEN];

bool search_tlb(lnaddr_t addr, hwaddr_t *h_addr);
void init_tlb();
