#include "riscv.h"

#ifndef _UART_REG_H_
#define _UART_REG_H_

#define           UART0_BASE	  0x310B0000	     //Size=  64K	 Max_offset=  0x00010000
#define           UART_BASE	  UART0_BASE          //Size=  64K	 Max_offset=  0x00010000
/*uart register definitions*/
#define SYNC_ADDR UART_BASE + 0xbc

#define RBR       UART_BASE + 0x00 + 0x10000*UART_NUM  
#define THR       UART_BASE + 0x00 + 0x10000*UART_NUM 
#define DLL       UART_BASE + 0x00 + 0x10000*UART_NUM 
#define DLH       UART_BASE + 0x04 + 0x10000*UART_NUM 
#define IER       UART_BASE + 0x04 + 0x10000*UART_NUM 
#define IIR       UART_BASE + 0x08 + 0x10000*UART_NUM 
#define FCR       UART_BASE + 0x08 + 0x10000*UART_NUM 
#define LCR       UART_BASE + 0x0c + 0x10000*UART_NUM 
#define MCR       UART_BASE + 0x10 + 0x10000*UART_NUM 
#define LSR       UART_BASE + 0x14 + 0x10000*UART_NUM 
#define MSR       UART_BASE + 0x18 + 0x10000*UART_NUM 
#define SCR_UART  UART_BASE + 0x1c + 0x10000*UART_NUM 
#define LPDLL     UART_BASE + 0x20 + 0x10000*UART_NUM 
#define LPDLH     UART_BASE + 0x24 + 0x10000*UART_NUM 
#define USR       UART_BASE + 0x7c + 0x10000*UART_NUM 
#define HTX       UART_BASE + 0xa4 + 0x10000*UART_NUM 
#define DMASA     UART_BASE + 0xa8 + 0x10000*UART_NUM 
#define UCV       UART_BASE + 0xf8 + 0x10000*UART_NUM 
#define CTR       UART_BASE + 0xfc + 0x10000*UART_NUM 

#define RFE  0x80
#define TEMT 0x40
#define THRE 0x20
#define BI   0x10
#define FE   0X08
#define PE   0X04
#define OE   0X02
#define DR   0X01
#define DTR  0X01
#define RTS  0X02
#define AFCE 0X20
#define SIRE 0X40
#define CTS  0X10

#endif

#define UART_NUM 0

#define REG_WRITE(addr, value)  (*((volatile unsigned int*)((unsigned long long)addr))) = (value)
#define REG_READ(addr)  (*((volatile unsigned int*)((unsigned long long)addr)))

void uart_delay(unsigned int loops)
{
  while(loops--){
    __asm__ volatile ("nop");
  }
}

// void printInfo(){
//   register unsigned int value asm("t0");
//   register unsigned int status asm("t1");

//   value = REG_READ(LSR);
//   status = !(value & 0x60);
//   //while(!(value & 0x60)) // transmit empty and transmit holding reg empty
//   while(status) // transmit empty and transmit holding reg empty
//   {
//     //uart_delay(100);
//     value = REG_READ(LSR);
//     status = !(value & 0x60);
//   }
//   //REG_WRITE(THR,data);
//   REG_WRITE(THR,0x54);

//   value = REG_READ(LSR);
//   status = !(value & 0x60);
//   //while(!(value & 0x60)) // transmit empty and transmit holding reg empty
//   while(status) // transmit empty and transmit holding reg empty
//   {
//     //uart_delay(100);
//     value = REG_READ(LSR);
//     status = !(value & 0x60);
//   }
//   //REG_WRITE(THR,data);
//   REG_WRITE(THR,0x44);
// }

void writechar(unsigned char data)
{
  unsigned int value;

  value = REG_READ(LSR);
  while(!(value & 0x60)) // transmit empty and transmit holding reg empty
  {
    uart_delay(100);
    value = REG_READ(LSR);
  }
  REG_WRITE(THR,data);
}

int readchar()
{
  unsigned int value;
  int recv;

  value = REG_READ(LSR);
  if (value & 0x1) {
    recv = REG_READ(RBR);
  }
  else {
    recv = -1;
  }

  return recv;
}

void initUart()
{
    REG_WRITE(LCR,0x83); //0x0c
    while((REG_READ(USR)&0x1) == 0x1);//0x7c
    REG_WRITE(DLH,0x0);   //0x04
    //REG_WRITE(DLL,0x44);   //0x00 125M/('h44*16) ~= 115200
    //REG_WRITE(DLL,0x36);   //0x00 100M/('h36*16) ~= 115200
    //REG_WRITE(DLL,0x4);   //0x00 100M/('h36*16) ~= 1562500
    //REG_WRITE(DLL,0x2);   //0x00 100M/('h36*16) ~=   3125000
    //REG_WRITE(DLL,0x82);   //0x00 20MHz/9600
    //REG_WRITE(DLL,0x6c);   //0x00 200MHz/115200/16
    //REG_WRITE(DLL,0x36);   //0x00 100MHz/115200/16
    REG_WRITE(DLL,0xa2);   //0x00 25MHz/9600/16
    //REG_WRITE(DLL,0x1b);   //0x00 50MHz/115200/16
    //REG_WRITE(DLL, 0x41);  //0x00 10MHz/9600/16
    //REG_WRITE(DLL, 0x82);  //0x00 20MHz/9600/16
    //REG_WRITE(DLL,0xD);  //0x00 2MHz/9600/16

    //REG_WRITE(DLL+0x10000*uart_num,0x1);   //0x00 100M/('h36*16) ~= 115200
    REG_WRITE(LCR,0x03);  //0x0c
    REG_WRITE(FCR,0x01);  //0x08  // enable fifo
    REG_WRITE(IER,0x0);  //0x04
    REG_WRITE(MCR,(RTS | DTR) );  //0x10
}

void __am_init_uartlite(void) {
  initUart();
}

void __am_uartlite_putchar(char ch) {
  if (ch == '\n') __am_uartlite_putchar('\r');
  writechar(ch);
}

int __am_uartlite_getchar() {
  return readchar();
}

// void copyAndRun(void)
// {
//   extern uint64_t reset_vector[];
//   extern uint64_t _run[];
//   // extern uint64_t _data[];
//   //extern uint64_t _end[];

//   #define ROM_START 0x10000000
//   #define RAM_START 0x80000000

//   // register uint64_t *romStart asm("t0") = (uint64_t *)(unsigned long long)ROM_START;
//   register uint64_t *ramStart asm("t1") = (uint64_t *)(unsigned long long)RAM_START;
//   register uint64_t *start asm("t2") = &reset_vector[0];
//   // register uint64_t *end asm("t3") = &_data[0];
//   //register uint64_t *end asm("t3") = &_end[0];
//   // uint64_t size = end - start;

//   // uint64_t i = 0;
//   // for (i = 0; i < size; i += 16) {
//   //    #define MACRO(j) ramStart[i + j] = romStart[i + j]
//   //    #define MACRO4(j) MACRO(j); MACRO(j + 1); MACRO(j + 2); MACRO(j + 3);
//   //    MACRO4(0); MACRO4(4); MACRO4(8); MACRO4(12);
//   // }

//   register uint64_t *run asm("t4") = &_run[0];
//   uint64_t runOffset = run - start;
//   register uint64_t *runAddr asm("t5") = ramStart + runOffset;
//   __asm__ __volatile__("fence.i");
//   (*(void(*) ())runAddr)();
// }


// void initBSS(void)
// {
//   #define RAM_START 0x80000000
//   uint64_t *ramStart  = (uint64_t *)(unsigned long long)RAM_START;

//   extern uint64_t reset_vector[];
//   extern uint64_t _bss_start[];
//   extern uint64_t _stack_top[];
//   uint64_t *bssStart = &_bss_start[0];
//   uint64_t *bssEnd   = &_stack_top[0];
//   uint64_t bssSize = bssEnd - bssStart;

//   uint64_t *codeStart = &reset_vector[0];
//   uint64_t codeSize = bssStart - codeStart;
//   ramStart += codeSize;
//   uint64_t i = 0;
//   for (i = 0; i < bssSize; i += 16) {
//      #define MEMSET(j) ramStart[i + j] = 0;
//      #define MEMSET4(j) MEMSET(j); MEMSET(j + 1); MEMSET(j + 2); MEMSET(j + 3);
//      MEMSET4(0); MEMSET4(4); MEMSET4(8); MEMSET4(12);
//   }
  
// }

// //-------------------------------------------
// #define READ  1
// #define WRITE 0
// #define MC    0
// #define PHY   1

// typedef struct {
//     uint8_t rw;
//     uint8_t mcPhy;
//     uint64_t addr;
//     uint32_t data;
// } DDRCmd;

// //mc  0x00_3106_0000-0x00_3106_FFFF
// //phy 0x00_3107_0000-0x00_3107_FFF
// DDRCmd cmds[] = 
// {
//   // mc config
//   {WRITE, MC,  0x31060304, 0x00000001}, // DBG1
//   {WRITE, MC,  0x31060030, 0x00000001}, // Self-Ref & Power Down
//   {WRITE, MC,  0x31060000, 0x40040610}, // 2T timing
//   {WRITE, MC,  0x31060010, 0x00000010}, // MR Control
//   {WRITE, MC,  0x31060014, 0x0000465c}, // MR Data
//   {WRITE, MC,  0x3106001c, 0xa2e0f445}, // MR PDA
//   {WRITE, MC,  0x31060030, 0x00000088}, // Self-Ref & Power Down
//   {WRITE, MC,  0x31060034, 0x000f0002}, // Self-Ref & Power Down Timing
//   {WRITE, MC,  0x31060038, 0x006a0000}, // Hardware Low Power
//   {WRITE, MC,  0x31060050, 0x00001060}, // 
//   {WRITE, MC,  0x31060060, 0x00000000},
//   {WRITE, MC,  0x31060064, 0x009200d3},
//   {WRITE, MC,  0x310600c0, 0x00000000},
//   {WRITE, MC,  0x310600c4, 0x00001010},
//   {WRITE, MC,  0x310600d0, 0x00020027},
//   {WRITE, MC,  0x310600d4, 0x00270006},
//   {WRITE, MC,  0x310600dc, 0x0a310001},
//   {WRITE, MC,  0x310600e0, 0x10180200},
//   {WRITE, MC,  0x310600e4, 0x00110000},
//   {WRITE, MC,  0x310600e8, 0x00000640},
//   {WRITE, MC,  0x310600ec, 0x00000800},
//   {WRITE, MC,  0x310600f0, 0x00000010},
//   {WRITE, MC,  0x31060100, 0x110d2914}, // rw2pre, t_faw, t_rax_max, t_ras_min
//   {WRITE, MC,  0x31060104, 0x0004051b}, // null, t_xp, rd2pre, t_rc
//   {WRITE, MC,  0x31060108, 0x06080510}, // wl, rl, 
//   {WRITE, MC,  0x3106010c, 0x0000f00f},
//   {WRITE, MC,  0x31060110, 0x08030309},
//   {WRITE, MC,  0x31060114, 0x0f0e0404},
//   {WRITE, MC,  0x31060120, 0x06060e08},
//   {WRITE, MC,  0x31060124, 0x0002020d},
//   {WRITE, MC,  0x31060128, 0x000f0e05},
//   {WRITE, MC,  0x3106012c, 0x14090112},
//   {WRITE, MC,  0x31060130, 0x0c00000d},
//   {WRITE, MC,  0x3106013c, 0x00000000},
//   {WRITE, MC,  0x31060180, 0x51000040},
//   {WRITE, MC,  0x31060184, 0x00000070},
//   {WRITE, MC,  0x31060190, 0x048b820a},
//   {WRITE, MC,  0x31060194, 0x01090606},
//   {WRITE, MC,  0x31060198, 0x07008120},
//   {WRITE, MC,  0x3106019c, 0x00000061},
//   {WRITE, MC,  0x310601a0, 0x20400004},
//   {WRITE, MC,  0x310601a4, 0x0003002e},
//   {WRITE, MC,  0x310601a8, 0x00000000},
//   {WRITE, MC,  0x310601b0, 0x00000041},
//   {WRITE, MC,  0x310601b8, 0x00000000},
//   {WRITE, MC,  0x310601c0, 0x00000001},
//   {WRITE, MC,  0x310601c4, 0x00000000},
//   {WRITE, MC,  0x31060204, 0x003f0007},
//   {WRITE, MC,  0x31060208, 0x03000400},
//   {WRITE, MC,  0x3106020c, 0x02020507},
//   {WRITE, MC,  0x31060210, 0x00001f1f},
//   {WRITE, MC,  0x31060214, 0x0b080709},
//   {WRITE, MC,  0x31060218, 0x0406080b},
//   {WRITE, MC,  0x3106021c, 0x00000f0f},
//   {WRITE, MC,  0x31060220, 0x00000204},
//   {WRITE, MC,  0x31060224, 0x02020107},
//   {WRITE, MC,  0x31060228, 0x0704080a},
//   {WRITE, MC,  0x3106022c, 0x00000007},
//   {WRITE, MC,  0x31060240, 0x0f1e0264},
//   {WRITE, MC,  0x31060244, 0x00000001},
//   {WRITE, MC,  0x31060250, 0x1aa51704},
//   {WRITE, MC,  0x31060254, 0x00000039},
//   {WRITE, MC,  0x3106025c, 0x40001e0b},
//   {WRITE, MC,  0x31060264, 0x2c006921},
//   {WRITE, MC,  0x3106026c, 0x79001d5c},
//   {WRITE, MC,  0x31060300, 0x00000001},
//   {WRITE, MC,  0x31060304, 0x00000000},
//   {WRITE, MC,  0x3106030c, 0x00000000},
//   {WRITE, MC,  0x31060320, 0x00000001},
//   {WRITE, MC,  0x31060328, 0x00000000},
//   {WRITE, MC,  0x3106036c, 0x00000010},
//   {WRITE, MC,  0x31060400, 0x00000010},
//   {WRITE, MC,  0x31060404, 0x000023e7},
//   {WRITE, MC,  0x31060408, 0x0000036e},
//   {WRITE, MC,  0x31060490, 0x00000000},
//   {WRITE, MC,  0x31060494, 0x00100005},
//   // mc reset done
//   //{READ ,0x31060ff4, 0x00000000},
//   {READ , MC,  0x31060ff4, 0x67612a2a}, //read only
//   // mc init
//   {WRITE, MC,  0x31060304, 0x00000000},
//   {WRITE, MC,  0x31060030, 0x00000088},
//   {WRITE, MC,  0x31060030, 0x00000088},
//   {WRITE, MC,  0x31060320, 0x00000000},
//   {WRITE, MC,  0x310601b0, 0x00000040},
//   // phy config
//   {WRITE, PHY, 0x31070024, 0x0020403b},
//   {WRITE, PHY, 0x31070148, 0x0f000009},
//   {WRITE, PHY, 0x31070149, 0x00000309},
//   {WRITE, PHY, 0x310701c4, 0x0e00003c},
//   {WRITE, PHY, 0x31070204, 0x0e00003c},
//   {WRITE, PHY, 0x31070244, 0x0e00003c},
//   {WRITE, PHY, 0x31070284, 0x0e00003c},
//   {WRITE, PHY, 0x310702c4, 0x0e00003c},
//   {WRITE, PHY, 0x31070304, 0x0e00003c},
//   {WRITE, PHY, 0x31070344, 0x0e00003c},
//   {WRITE, PHY, 0x31070384, 0x0e00003c},
//   {WRITE, PHY, 0x31070080, 0x3000b0c7},
//   {WRITE, PHY, 0x3107014b, 0x0fc00172},
//   {WRITE, PHY, 0x31070040, 0x2800040c},
//   {WRITE, PHY, 0x31070022, 0x20c01884},
//   {WRITE, PHY, 0x31070010, 0x5e001810},
//   {WRITE, PHY, 0x31070011, 0x008005f0},
//   {WRITE, PHY, 0x31070060, 0x00000a31},
//   {WRITE, PHY, 0x31070061, 0x00000001},
//   {WRITE, PHY, 0x31070062, 0x00000018},
//   {WRITE, PHY, 0x31070063, 0x00000200},
//   {WRITE, PHY, 0x31070064, 0x00000000},
//   {WRITE, PHY, 0x31070065, 0x00000640},
//   {WRITE, PHY, 0x31070080, 0x3000b0c7},
//   {WRITE, PHY, 0x31070066, 0x00000800},
//   {WRITE, PHY, 0x31070044, 0x0627100a},
//   {WRITE, PHY, 0x31070045, 0x281a0408},
//   {WRITE, PHY, 0x31070046, 0x00070380},
//   {WRITE, PHY, 0x31070047, 0x02000101},
//   {WRITE, PHY, 0x31070048, 0x01cd0608},
//   {WRITE, PHY, 0x31070049, 0x0036100a},
//   {WRITE, PHY, 0x31070005, 0x020046a0},
//   {WRITE, PHY, 0x31070006, 0x20006c2b},
//   {WRITE, PHY, 0x31070007, 0xc0aa0060},
//   {WRITE, PHY, 0x3107000b, 0x80040000},
//   {WRITE, PHY, 0x31070001, 0x00000073},
//   // phy init
//   {READ , PHY, 0x3107000d, 0x8000000f},
//   // phy SDRAM config
//   {WRITE, PHY, 0x31070001, 0x00040001},
//   // phy SDRAM init
//   {READ , PHY, 0x3107000d, 0x8000001f},
//   // mc SDRAM config
//   {WRITE, MC,  0x310601b0, 0x00000041},
//   {WRITE, MC,  0x31060320, 0x00000001},
//   // mc SDRAM init(!!!!!!)
//   {READ , MC,  0x31060004, 0x00000001},  // this cannot pass
//   // mc SDRAM config reset
//   {WRITE, MC,  0x31060320, 0x00000000},
//   {WRITE, MC,  0x31060060, 0x00000001},
//   {WRITE, MC,  0x310601b0, 0x00000040},
//   {WRITE, MC,  0x31060320, 0x00000001},
//   {WRITE, MC,  0x31060010, 0x00001010},
//   {WRITE, MC,  0x31060014, 0x00000181},
//   {WRITE, MC,  0x31060010, 0x80001010},
//   // write leveling config
//   {WRITE, PHY, 0x310700c5, 0x000000ff},
//   {WRITE, PHY, 0x31070137, 0x00000000},
//   {WRITE, PHY, 0x31070026, 0x00010000},
//   {WRITE, PHY, 0x31070001, 0x00000201},
//   // write leveling training
//   {READ , PHY, 0x3107000d, 0x8000003f},
//   // write leveling config reset
//   {WRITE, MC,  0x31060010, 0x00001010},
//   {WRITE, MC,  0x31060014, 0x00000101},
//   {WRITE, MC,  0x31060010, 0x80001010},
//   // read & write training config
//   {WRITE, PHY, 0x31070001, 0x0002fc01},
//   // read & write training
//   {READ , PHY, 0x3107000d, 0x80004fff},
//   {WRITE, MC,  0x31060320, 0x00000000},
//   {WRITE, MC,  0x31060060, 0x00000000},
//   {WRITE, MC,  0x31060320, 0x00000001},
//   {WRITE, MC,  0x31060490, 0x00000001}
// };

// void initDDR(void)
// {
//     register int i asm ("t0");
//     uint32_t rdata;
//     for (i = 0; i < sizeof(cmds) / sizeof(DDRCmd); i++) {
//         register DDRCmd *p asm ("t1") = &cmds[i];
//         uint64_t addrValue = p->addr;
//         if(p->mcPhy==PHY)
//           addrValue = (addrValue & 0xffff0000) | (((addrValue & 0x0000ffff) << 2) & 0x0000ffff);
//         volatile uint32_t *addr = (uint32_t *)(addrValue);
//         if (p->rw == READ) {
//             rdata = *addr;
//             while(rdata != p->data) // LSR = 0x1, data ready
//             {
//               rdata = *addr;
//             }
//         }
//         else {
//             *addr = p->data;
//         }
//     }

//     //write_csr(0x800, 1);
// }
