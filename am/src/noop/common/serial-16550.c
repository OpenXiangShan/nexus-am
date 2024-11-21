#include <riscv.h>


/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


//#define SERIAL_MMIO 0x10000000
#define SERIAL_MMIO 0x310b0000

#define RBR       0x00
#define THR       0x00
#define DLL       0x00
#define DLH       0x04
#define IER       0x04
#define FCR       0x08
#define LCR       0x0c
#define MCR       0x10
#define LSR       0x14
#define USR       0x7c

#define DTR  0X01
#define RTS  0X02

// note: ns16550a_delay __am_16550_putchar __am_init_16550 is migrated directly from https://github.com/OpenXiangShan/gos/blob/master/mysbi/sbi/uart_ns16550a.c

static void ns16550a_delay(unsigned int loops)
{
  while (loops--) {
    __asm__ volatile ("nop");
  }
}

void __am_16550_putchar(char ch) {
  if (ch == '\n') __am_16550_putchar('\r');

  unsigned int value;

  value = inl(SERIAL_MMIO + LSR);
  while (!(value & 0x60)) {
    ns16550a_delay(100);
    value = inl(SERIAL_MMIO + LSR);
  }

  outl(SERIAL_MMIO + THR, ch);

}

void __am_init_16550(){
  unsigned int divisor;
  divisor = 50000000 / (16 * 115200);

  outl(SERIAL_MMIO + IER, 0x00);

  outl(SERIAL_MMIO + LCR, 0x83); // enable DLAB
	while (inl(SERIAL_MMIO + USR) & 0x1) ;

  outl(SERIAL_MMIO + DLH, 0x00); // 115200
  outl(SERIAL_MMIO + DLL, divisor); // 115200

  outl(SERIAL_MMIO + LCR, 0x03); // disable DLAB

  outl(SERIAL_MMIO + FCR, 0x01);
  outl(SERIAL_MMIO + MCR, RTS | DTR);
}
