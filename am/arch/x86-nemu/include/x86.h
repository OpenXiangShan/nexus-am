#ifndef __X86_H__
#define __X86_H__

// Eflags register
#define FL_TF     0x00000100  // Trap Flag
#define FL_IF     0x00000200  // Interrupt Enable

// Control Register flags
#define CR0_PE    0x00000001  // Protection Enable
#define CR0_PG    0x80000000  // Paging

// Page directory and page table constants
#define NR_PDE    1024    // # directory entries per page directory
#define NR_PTE    1024    // # PTEs per page table
#define PGSHFT    12      // log2(PGSIZE)
#define PTXSHFT   12      // Offset of PTX in a linear address
#define PDXSHFT   22      // Offset of PDX in a linear address

// Page table/directory entry flags
#define PTE_P     0x001     // Present
#define PTE_W     0x002     // Writeable
#define PTE_U     0x004     // User
#define PTE_PWT   0x008     // Write-Through
#define PTE_PCD   0x010     // Cache-Disable
#define PTE_A     0x020     // Accessed
#define PTE_D     0x040     // Dirty

// IDT size
#define NR_IRQ    256     // IDT size

// interrupts
#define T_IRQ0 32
#define IRQ_TIMER 0
#define IRQ_KBD   1
#define IRQ_SPURIOUS 31
#define IRQ_ERROR 19


// The following macros will not be seen by the assembler
#ifndef __ASSEMBLER__

#include <arch.h>

// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/\------ OFF(va) ------/
typedef uint32_t PTE;
typedef uint32_t PDE;
#define PDX(va)     (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHFT | (t) << PTXSHFT | (o)))

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)

// Gate descriptors for interrupts and traps
typedef struct GateDesc {
  uint32_t off_15_0 : 16;   // Low 16 bits of offset in segment
  uint32_t cs : 16;     // Code segment selector
  uint32_t args : 5;    // # args, 0 for interrupt/trap gates
  uint32_t rsv1 : 3;    // Reserved(should be zero I guess)
  uint32_t type : 4;    // Type(STS_{TG,IG32,TG32})
  uint32_t s : 1;       // Must be 0 (system)
  uint32_t dpl : 2;     // Descriptor(meaning new) privilege level
  uint32_t p : 1;       // Present
  uint32_t off_31_16 : 16;  // High bits of offset in segment
} GateDesc;

#define GATE(type, cs, entry, dpl) (GateDesc)         \
{  (uint32_t)(entry) & 0xffff, (cs), 0, 0, (type), 0, (dpl), \
  1, (uint32_t)(entry) >> 16 }

static inline void cli() {
  asm volatile("cli");
}

static inline void sti() {
  asm volatile("sti");
}

static inline void hlt() {
  asm volatile("hlt");
}

static inline uint32_t get_efl() {
  volatile uint32_t efl;
  asm volatile("pushf; pop %0": "=r"(efl));
  return efl;
}

static inline uint32_t get_cr0(void) {
  volatile uint32_t val;
  asm volatile("movl %%cr0, %0" : "=r"(val));
  return val;
}

static inline void set_cr0(uint32_t cr0) {
  asm volatile("movl %0, %%cr0" : : "r"(cr0));
}


static inline void set_idt(GateDesc *idt, int size) {
  volatile static uint16_t data[3];
  data[0] = size - 1;
  data[1] = (uint32_t)idt;
  data[2] = (uint32_t)idt >> 16;
  asm volatile("lidt (%0)" : : "r"(data));
}

static inline uint32_t get_cr2() {
  volatile uint32_t val;
  asm volatile("movl %%cr2, %0" : "=r"(val));
  return val;
}

static inline void set_cr3(void *pdir) {
  asm volatile("movl %0, %%cr3" : : "r"(pdir));
}


#endif

#endif
