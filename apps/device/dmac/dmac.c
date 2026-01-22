#include <klib.h>

#define DMAC_BASE_ADDR 0x40003000


#define DMAC_SRC_ADDR_REG       (DMAC_BASE_ADDR + 0x00)
#define DMAC_DST_ADDR_REG       (DMAC_BASE_ADDR + 0x08)
#define DMAC_CTRL_REG           (DMAC_BASE_ADDR + 0x10)
                                
#define Test_SRC_ADDR           0x100000000 //4G
#define Test_DST_ADDR           0x800000000 //32G
#define Test_PATTERN            0x5A5A5A5A5A5A5A5A

// cache flush
static inline void cache_flush_addr(uint64_t addr) {
    asm volatile (
        "fence w,w\n\t"          
        "cbo.flush (%0)\n\t"   
        "fence rw,rw\n\t"  
        :
        : "r"(addr)
        : "memory"
    );
}

// cache invalid
static inline void cache_inval_addr(uint64_t addr) {
    asm volatile (
        "fence w,w\n\t"          
        "cbo.inval (%0)\n\t"   
        "fence rw,rw\n\t"  
        :
        : "r"(addr)
        : "memory"
    );
}

int main()
{
    printf("Hello, DMA!\n");

    // init dmac
    *(volatile uint64_t*)DMAC_SRC_ADDR_REG = Test_SRC_ADDR;
    *(volatile uint64_t*)DMAC_DST_ADDR_REG = Test_DST_ADDR;
    *(volatile uint64_t*)Test_SRC_ADDR = Test_PATTERN;
    
    // flush cache line to ddr
    cache_flush_addr(Test_SRC_ADDR);

    // start dmac transfer once
    *(volatile uint64_t*)DMAC_CTRL_REG = 0x1;
    /* */
    // waiting for transfer done
    uint64_t ctrl_reg;
    uint64_t timeer = 0;
    while(1){
        // readback DMAC_CTRL_REG
        ctrl_reg = *(volatile uint64_t*)DMAC_CTRL_REG;
        if ((ctrl_reg >> 1) == 1){ // transfer done
            // cache invalid
            cache_inval_addr(Test_DST_ADDR);
            if(*(volatile uint64_t*)Test_DST_ADDR == Test_PATTERN){ // read back compare with pattern
                printf("DMA Test Success!\n");
            }else{
                printf("DMA Test failed with pattern compare!\n");
                printf("Pattern value: 0x%lx\n", Test_PATTERN);
                printf("Read back value: 0x%lx\n", *(volatile uint64_t*)Test_DST_ADDR);
            }
            break;
        }
        if (timeer++ > 4096){ // timeout watchdog
            printf("DMA Test failed with timeout!\n");
            break;
        }
    }  
    return 0;
}