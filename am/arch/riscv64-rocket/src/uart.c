#include <klib.h>
#include "fdt.h"

volatile uint8_t* uart_base_ptr;

#define UART_RX_FIFO_REG 0
#define UART_TX_FIFO_REG 0x4
#define UART_STAT_REG 0x8
#define UART_CTRL_REG 0xc

void uart_send(uint8_t data) {
	// wait until THR empty
	while((*(uart_base_ptr + UART_STAT_REG) & 0x08));
	*(uart_base_ptr + UART_TX_FIFO_REG) = data;
}

void uart_send_string(const char *str) {
	while(*str != 0) {
		uart_send(*(str++));
	}
}

void uart_send_buf(const char *buf, const int32_t len) {
	int32_t i;
	for(i=0; i<len; i++) {
		uart_send(buf[i]);
	}
}

int uart_recv() {
	// check whether RBR has data
	if(! (*(uart_base_ptr + UART_STAT_REG) & 0x01u)) {
		return -1;
	}
	return *(uart_base_ptr + UART_RX_FIFO_REG);
}

// IRQ triggered read
uint8_t uart_read_irq() {
	return *(uart_base_ptr + UART_RX_FIFO_REG);
}

// check uart IRQ for read
uint8_t uart_check_read_irq() {
	return (*(uart_base_ptr + UART_STAT_REG) & 0x01u);
}

// enable uart read IRQ
void uart_enable_read_irq() {
	*(uart_base_ptr + UART_CTRL_REG) = 0x0010u;
}

// disable uart read IRQ
void uart_disable_read_irq() {
	*(uart_base_ptr + UART_CTRL_REG) = 0x0000u;
}

struct uart_scan
{
  int compat;
  uint64_t reg;
};

static void uart_open(const struct fdt_scan_node *node, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  memset(scan, 0, sizeof(*scan));
}

static void uart_prop(const struct fdt_scan_prop *prop, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  if (!strcmp(prop->name, "compatible") && !strcmp((const char*)prop->value, "sifive,uart0")) {
    scan->compat = 1;
  } else if (!strcmp(prop->name, "reg")) {
    fdt_get_address(prop->node->parent, prop->value, &scan->reg);
  }
}

static void uart_done(const struct fdt_scan_node *node, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  if (!scan->compat || !scan->reg) return;

  // Enable Rx/Tx channels
  uart_base_ptr = (void*)scan->reg;
}

void uart_init(uintptr_t fdt)
{
  struct fdt_cb cb;
  struct uart_scan scan;

  memset(&cb, 0, sizeof(cb));
  cb.open = uart_open;
  cb.prop = uart_prop;
  cb.done = uart_done;
  cb.extra = &scan;

  fdt_scan(fdt, &cb);

  if(!uart_base_ptr) {
	  //uart_base_ptr = (void *)0x60000000;
    return;
  }
  /*

  uart_base_ptr += read_const_csr(mhartid) * 0x10000;
  */

  // reset the receive FIFO and transmit FIFO
  *(uart_base_ptr + UART_CTRL_REG) = 0x3;
}
