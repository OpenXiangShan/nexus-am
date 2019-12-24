#include <riscv64.h>

/*
#define UARTLITE_MMIO 0x40600000
#define UARTLITE_RX_FIFO  0x0
#define UARTLITE_TX_FIFO  0x4
#define UARTLITE_STAT_REG 0x8
#define UARTLITE_CTRL_REG 0xc

#define UARTLITE_RST_FIFO 0x03
#define UARTLITE_TX_FULL  0x08
#define UARTLITE_RX_VALID 0x01

void __am_init_uartlite(void) {
  outb(UARTLITE_MMIO + UARTLITE_CTRL_REG, UARTLITE_RST_FIFO);
}

void __am_uartlite_putchar(char ch) {
  if (ch == '\n') __am_uartlite_putchar('\r');

  while (inb(UARTLITE_MMIO + UARTLITE_STAT_REG) & UARTLITE_TX_FULL);
  outb(UARTLITE_MMIO + UARTLITE_TX_FIFO, ch);
}

int __am_uartlite_getchar() {
  if (inb(UARTLITE_MMIO + UARTLITE_STAT_REG) & UARTLITE_RX_VALID)
    return inb(UARTLITE_MMIO + UARTLITE_RX_FIFO);
  return 0;
}
*/
#define FREQ 50000000 //50MHz input clock
#define BAUD 115200
#define DIV  434 //FREQ/(BAUD*16)
#define UART_BASE 	0x41000000

#define REG8(add)  *((volatile unsigned char *)  (add))
#define REG16(add) *((volatile unsigned short *) (add))
#define REG32(add) *((volatile unsigned int*)  (add))  //rv64

/*****the uart register is little-endian and 4 byte-align******/

#define RB_THR_OFF  0x00 + 0x00
#define IER_OFF     0x04 + 0x00
#define IIR_FCR_OFF 0x08 + 0x00
#define LCR_OFF     0x0c + 0x00
#define MCR_OFF     0x10 + 0x00
#define LSR_OFF     0x14 + 0x00
#define MSR_OFF     0x18 + 0x00
#define CDRl_OFF    0x00 + 0x00
#define CDRh_OFF    0x04 + 0x01

#define DLAB_EN   0x83
#define DLAB_DIS  0x03

#define DL 0x001b
#define DL_1 0x1b
#define DL_2 0x00

void
uart_set_baud(){
    
    //unsigned int DL = FREQ/(BAUD*16);
    //unsigned int DL_1 = DL & 0x000000ff;   //LSB of DL 
    //unsigned int DL_2 = DL & 0x0000ff00;   //MSB of DL
    //REG8(UART_BASE+LCR_OFF)  = 0x33221100 | DLAB_EN; //enable DLAB
    //REG(UART_BASE+CDRl_OFF) = 0x44332200 | DL_1; //dl[`UART_DL1]=0x1b
    //REG32(UART_BASE+CDRh_OFF) = 0x44330011 | DL_2; //dl[`UART_DL2]=0x00
    //REG32(UART_BASE+LCR_OFF)  = 0x33221100 | DLAB_DIS; //disable DLAB
   REG8(UART_BASE+0x0c+0x03) = 0x83; //enable DLAB
   REG8(UART_BASE+0x00+0x00) = 0x1b; //dl[`UART_DL1]=0x1b
   REG8(UART_BASE+0x04+0x01) = 0x00; //dl[`UART_DL2]=0x00
   REG8(UART_BASE+0x0c+0x03) = 0x03; //disable DLAB
}

void __am_init_uartlite(void) {
    uart_set_baud();
    //REG8(UART_BASE+IIR_FCR_OFF) = 0x40;
    //REG8(UART_BASE+IER_OFF) = 0x01;
}

void __am_uartlite_putchar(char ch) {
    if (ch == '\n') __am_uartlite_putchar('\r');

    while(!(REG8(UART_BASE+LSR_OFF) & 0x20));
    //wait until transmitter FIFO is empty
    //judge condition LSR[5]==1
    REG8(UART_BASE+RB_THR_OFF) = ch;
    
}

int __am_uartlite_getchar() {
    while(!(REG8(UART_BASE+LSR_OFF) & 0x1));
    //wait until a character has been recceived
    //judge condition LSR[0]==1
    return (unsigned int)REG8(UART_BASE+RB_THR_OFF);
}
