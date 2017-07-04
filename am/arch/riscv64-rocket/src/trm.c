#include <am.h>
#include <klib.h>
#include "fdt.h"

uintptr_t fdt = 0;
static uintptr_t mem_size = 0;

extern char _end;
extern int main();
extern void uart_init(uintptr_t fdt);
extern void uart_send(uint8_t data);

_Area _heap = {
  .start = &_end,
};

//////////////////////////////////////////// MEMORY SCAN /////////////////////////////////////////

struct mem_scan {
  int memory;
  const uint32_t *reg_value;
  int reg_len;
};

static void mem_open(const struct fdt_scan_node *node, void *extra)
{
  struct mem_scan *scan = (struct mem_scan *)extra;
  memset(scan, 0, sizeof(*scan));
}

static void mem_prop(const struct fdt_scan_prop *prop, void *extra)
{
  struct mem_scan *scan = (struct mem_scan *)extra;
  if (!strcmp(prop->name, "device_type") && !strcmp((const char*)prop->value, "memory")) {
    scan->memory = 1;
  } else if (!strcmp(prop->name, "reg")) {
    scan->reg_value = prop->value;
    scan->reg_len = prop->len;
  }
}

static void mem_done(const struct fdt_scan_node *node, void *extra)
{
  struct mem_scan *scan = (struct mem_scan *)extra;
  const uint32_t *value = scan->reg_value;
  const uint32_t *end = value + scan->reg_len/4;
  uintptr_t self = (uintptr_t)mem_done;

  if (!scan->memory) return;
  assert (scan->reg_value && scan->reg_len % 4 == 0);

  while (end - value > 0) {
    uint64_t base, size;
    value = fdt_get_address(node->parent, value, &base);
    value = fdt_get_size   (node->parent, value, &size);
    if (base <= self && self <= base + size) { mem_size = size; }
  }
  assert (end == value);
}

void query_mem(uintptr_t fdt)
{
  struct fdt_cb cb;
  struct mem_scan scan;

  memset(&cb, 0, sizeof(cb));
  cb.open = mem_open;
  cb.prop = mem_prop;
  cb.done = mem_done;
  cb.extra = &scan;

  mem_size = 0;
  fdt_scan(fdt, &cb);
  assert (mem_size > 0);
}

void _putc(char ch) {
  uart_send(ch);
}

static void puts(const char *s) {
  for (const char *p = s; *p; p ++) {
    _putc(*p);
  }
}

void _halt(int code) {
  puts("Exited (");
  if (code == 0) _putc('0');
  else {
    char buf[10], *p = buf + 9;
    for (*p = 0; code; code /= 10) {
      *(-- p) = '0' + code % 10;
    }
    puts(p);
  }
  puts(").\n");

  while (1);
}

void _trm_init() {
  uart_init(fdt);

  query_mem(fdt);
  _heap.end = (void *)0x80000000 + mem_size;

  int ret = main();
  _halt(ret);
}
