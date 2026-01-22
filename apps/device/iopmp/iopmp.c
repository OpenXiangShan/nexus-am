#include <klib.h>

// DMAC
#define DMAC_BASE_ADDR 0x40003000

#define DMAC_SRC_ADDR_REG       (DMAC_BASE_ADDR + 0x00)
#define DMAC_DST_ADDR_REG       (DMAC_BASE_ADDR + 0x08)
#define DMAC_CTRL_REG           (DMAC_BASE_ADDR + 0x10)
                                
#define Test_SRC_ADDR           0x100000000 //4G
#define Test_DST_ADDR           0x800000000 //32G
#define Test_PATTERN            0x5A5A5A5A5A5A5A5A

//IOPMP
#define IOPMP_BASE_ADDR 0x40100000

#define IOPMP_HWCFG0_REG       (IOPMP_BASE_ADDR + 0x08)
#define IOPMP_ERRCFG_REG       (IOPMP_BASE_ADDR + 0x60)
#define IOPMP_ERRINF_REG       (IOPMP_BASE_ADDR + 0x64)
#define IOPMP_SRCMD_BASE       (IOPMP_BASE_ADDR + 0x1000)
#define IOPMP_MDCFG_BASE       (IOPMP_BASE_ADDR + 0x800)
#define IOPMP_ENTRY_ADDR_BASE  (IOPMP_BASE_ADDR + 0x2000)
#define IOPMP_ENTRY_ADDRH_BASE (IOPMP_BASE_ADDR + 0x2004)
#define IOPMP_ENTRY_CFG_BASE   (IOPMP_BASE_ADDR + 0x2008)

#define IOPMP_SRCMD_STRIDE     0x20
#define IOPMP_MDCFG_STRIDE     0x04
#define IOPMP_ENTRY_STRIDE     0x10

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

/*  
    IOPMP bypass test 
    Disable IOPMP and test DMAC only
*/
void iopmp_test_bypass()
{
    printf("IOPMP bypass test begin\n"); 
    // init dmac
    *(volatile uint64_t*)DMAC_SRC_ADDR_REG = Test_SRC_ADDR;
    *(volatile uint64_t*)DMAC_DST_ADDR_REG = Test_DST_ADDR;
    *(volatile uint64_t*)Test_SRC_ADDR = Test_PATTERN;
    *(volatile uint64_t*)Test_DST_ADDR = 0x0000000000000000;
    
    // flush cache line to ddr
    cache_flush_addr(Test_SRC_ADDR);
    cache_flush_addr(Test_DST_ADDR);

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
                printf("IOPMP bypass test success!\n");
            }else{
                printf("IOPMP bypass test failed with pattern compare!\n");
                printf("Pattern value: 0x%lx\n", Test_PATTERN);
                printf("Read back value: 0x%lx\n", *(volatile uint64_t*)Test_DST_ADDR);
            }
            break;
        }
        if (timeer++ > 4096){ // timeout watchdog
            printf("IOPMP bypass test failed with timeout!\n");
            break;
        }
    }  
}

/*  
    IOPMP checkpass test 
    Enable iopmp and test DMAC and check if DMAC trans is check passed
*/
void iopmp_test_checkpass()
{
    printf("IOPMP checkpass test begin\n"); 
    // init dmac
    *(volatile uint64_t*)DMAC_SRC_ADDR_REG = Test_SRC_ADDR;
    *(volatile uint64_t*)DMAC_DST_ADDR_REG = Test_DST_ADDR;
    *(volatile uint64_t*)Test_SRC_ADDR = Test_PATTERN;
    *(volatile uint64_t*)Test_DST_ADDR = 0x0000000000000000;

    // flush cache line to ddr
    cache_flush_addr(Test_SRC_ADDR);
    cache_flush_addr(Test_DST_ADDR);

    //init iopmp
    *(volatile uint32_t*)IOPMP_HWCFG0_REG = 0x80000000; // enable iopmp
    *(volatile uint32_t*)IOPMP_ERRCFG_REG = 0x00000060; // enable error interrupt
    *(volatile uint32_t*)IOPMP_SRCMD_BASE = 0x00000002; // cfg srcmd table [0] index to mdcfg[0]
    *(volatile uint32_t*)IOPMP_MDCFG_BASE = 0x00000001; // cfg mdcfg[0] index to 1 entry,entry[0]

    *(volatile uint32_t*)IOPMP_ENTRY_ADDR_BASE  = 0xFFFFFFFF; // cfg entry[0]_addr // 512G addr range
    *(volatile uint32_t*)IOPMP_ENTRY_ADDRH_BASE = 0x0000000F; // cfg entry[0]_addh
    *(volatile uint32_t*)IOPMP_ENTRY_CFG_BASE   = 0x0000001F; // cfg entry[0]_cfg R=1 W=1 (Full access)

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
                printf("IOPMP checkpass test success!\n");
            }else{
                printf("IOPMP checkpass test failed with pattern compare!\n");
                printf("Pattern value: 0x%lx\n", Test_PATTERN);
                printf("Read back value: 0x%lx\n", *(volatile uint64_t*)Test_DST_ADDR);
            }
            break;
        }
        if (timeer++ > 4096){ // timeout watchdog
            printf("IOPMP checkpass test failed with timeout!\n");
            break;
        }
    }  
}

/*  
    IOPMP checkpass test 
    Enable iopmp and test DMAC and check if DMAC trans is check denined
*/
void iopmp_test_checkdenined()
{
    printf("IOPMP checkdenined test begin\n"); 
    // init dmac
    *(volatile uint64_t*)DMAC_SRC_ADDR_REG = Test_SRC_ADDR;
    *(volatile uint64_t*)DMAC_DST_ADDR_REG = Test_DST_ADDR;
    *(volatile uint64_t*)Test_SRC_ADDR = Test_PATTERN;
    *(volatile uint64_t*)Test_DST_ADDR = 0x0000000000000000;

    // flush cache line to ddr
    cache_flush_addr(Test_SRC_ADDR);
    cache_flush_addr(Test_DST_ADDR);

    //init iopmp
    *(volatile uint32_t*)IOPMP_HWCFG0_REG = 0x80000000; // enable iopmp
    *(volatile uint32_t*)IOPMP_ERRCFG_REG = 0x00000060; // enable error interrupt
    *(volatile uint32_t*)IOPMP_SRCMD_BASE = 0x00000002; // cfg srcmd table [0] index to mdcfg[0]
    *(volatile uint32_t*)IOPMP_MDCFG_BASE = 0x00000001; // cfg mdcfg[0] index to 1 entry,entry[0]

    *(volatile uint32_t*)IOPMP_ENTRY_ADDR_BASE  = 0xFFFFFFFF; // cfg entry[0]_addr // 512G addr range
    *(volatile uint32_t*)IOPMP_ENTRY_ADDRH_BASE = 0x0000000F; // cfg entry[0]_addh
    *(volatile uint32_t*)IOPMP_ENTRY_CFG_BASE   = 0x0000001C; // cfg entry[0]_cfg R=0 W=0 (Full denined)

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
             // read back compare with pattern and check error interrupt
            if(*(volatile uint64_t*)Test_DST_ADDR != Test_PATTERN && (*(volatile uint32_t*)IOPMP_ERRINF_REG & 1) == 1){
                printf("IOPMP checkdenined test success!\n");
            }else{
                printf("IOPMP checkdenined test failed with pattern compare and error interrupt!\n");
                printf("Pattern value: 0x%lx\n", Test_PATTERN);
                printf("Read back value: 0x%lx\n", *(volatile uint64_t*)Test_DST_ADDR);
                printf("Error info value: 0x%x\n", *(volatile uint32_t*)IOPMP_ERRINF_REG);
            }
            break;
        }
        if (timeer++ > 4096){ // timeout watchdog
            printf("IOPMP checkdenined test failed with timeout!\n");
            break;
        }
    }  
}



int main()
{
    printf("Hello, IOPMP!\n");
    iopmp_test_bypass();
    iopmp_test_checkpass();
    iopmp_test_checkdenined();

    return 0;
}