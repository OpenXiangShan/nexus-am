#ifndef __X86_H__
#define __X86_H__

// CPU rings
#define DPL_KERN  0x0     // Kernel (ring 0)
#define DPL_USER  0x3     // User (ring 3)

// Application segment type bits
#define STA_X     0x8     // Executable segment
#define STA_W     0x2     // Writeable (non-executable segments)
#define STA_R     0x2     // Readable (executable segments)

// System segment type bits
#define STS_T32A  0x9     // Available 32-bit TSS
#define STS_IG32  0xE     // 32-bit Interrupt Gate
#define STS_TG32  0xF     // 32-bit Trap Gate

// Eflags register
#define FL_TF     0x00000100  // Trap Flag
#define FL_IF     0x00000200  // Interrupt Enable

// Control Register flags
#define CR0_PE    0x00000001  // Protection Enable
#define CR0_PG    0x80000000  // Paging

// Page directory and page table constants
#define PGSIZE    4096    // Bytes mapped by a page
#define PGMASK    4095    // Mask for bit ops
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

// GDT entries
#define NR_SEG    6       // GDT size
#define SEG_KCODE   1       // Kernel code
#define SEG_KDATA   2       // Kernel data/stack
#define SEG_UCODE   3       // User code
#define SEG_UDATA   4       // User data/stack
#define SEG_TSS   5       // Global unique task state segement

#define KSEL(desc) (((desc) << 3) | DPL_KERN)
#define USEL(desc) (((desc) << 3) | DPL_USER)

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
typedef u32 PTE;
typedef u32 PDE;
#define PDX(va)     (((u32)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((u32)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((u32)(va) & 0xfff)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((u32)((d) << PDXSHFT | (t) << PTXSHFT | (o)))

#define PGROUNDUP(sz)   (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a)  (((a)) & ~(PGSIZE-1))

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((u32)(pte) & ~0xfff)

// Segment Descriptor
typedef struct SegDesc {
  u32 lim_15_0 : 16;  // Low bits of segment limit
  u32 base_15_0 : 16; // Low bits of segment base address
  u32 base_23_16 : 8; // Middle bits of segment base address
  u32 type : 4;     // Segment type (see STS_ constants)
  u32 s : 1;      // 0 = system, 1 = application
  u32 dpl : 2;    // Descriptor Privilege Level
  u32 p : 1;      // Present
  u32 lim_19_16 : 4;  // High bits of segment limit
  u32 avl : 1;    // Unused (available for software use)
  u32 rsv1 : 1;     // Reserved
  u32 db : 1;     // 0 = 16-bit segment, 1 = 32-bit segment
  u32 g : 1;      // Granularity: limit scaled by 4K when set
  u32 base_31_24 : 8; // High bits of segment base address
} SegDesc;

#define SEG(type, base, lim, dpl) (SegDesc)           \
{  ((lim) >> 12) & 0xffff, (u32)(base) & 0xffff,    \
  ((u32)(base) >> 16) & 0xff, type, 1, dpl, 1,     \
  (u32)(lim) >> 28, 0, 0, 1, 1, (u32)(base) >> 24 }

#define SEG16(type, base, lim, dpl) (SegDesc)         \
{  (lim) & 0xffff, (u32)(base) & 0xffff,        \
  ((u32)(base) >> 16) & 0xff, type, 0, dpl, 1,     \
  (u32)(lim) >> 16, 0, 0, 1, 0, (u32)(base) >> 24 }

// Gate descriptors for interrupts and traps
typedef struct GateDesc {
  u32 off_15_0 : 16;   // Low 16 bits of offset in segment
  u32 cs : 16;     // Code segment selector
  u32 args : 5;    // # args, 0 for interrupt/trap gates
  u32 rsv1 : 3;    // Reserved(should be zero I guess)
  u32 type : 4;    // Type(STS_{TG,IG32,TG32})
  u32 s : 1;       // Must be 0 (system)
  u32 dpl : 2;     // Descriptor(meaning new) privilege level
  u32 p : 1;       // Present
  u32 off_31_16 : 16;  // High bits of offset in segment
} GateDesc;

#define GATE(type, cs, entry, dpl) (GateDesc)         \
{  (u32)(entry) & 0xffff, (cs), 0, 0, (type), 0, (dpl), \
  1, (u32)(entry) >> 16 }


// Task state segment format
struct TSS {
  u32 link;     // Unused
  u32 esp0;     // Stack pointers and segment selectors
  u32 ss0;      //   after an increase in privilege level
  char dontcare[88];
};

static inline u8 inb(int port) {
  char data;
  asm volatile("inb %1, %0" : "=a"(data) : "d"((u16)port));
  return data;
}

static inline u32 inl(int port) {
  long data;
  asm volatile("inl %1, %0" : "=a"(data) : "d"((u16)port));
  return data;
}

static inline void outb(int port, int data) {
  asm volatile("outb %%al, %%dx" : : "a"(data), "d"((u16)port));
}

static inline void cli() {
  asm volatile("cli");
}

static inline void sti() {
  asm volatile("sti");
}

static inline void hlt() {
  asm volatile("hlt");
}

static inline u32 get_efl() {
  volatile u32 efl;
  asm volatile("pushf; pop %0": "=r"(efl));
  return efl;
}

static inline u32 get_cr0(void) {
  volatile u32 val;
  asm volatile("movl %%cr0, %0" : "=r"(val));
  return val;
}

static inline void set_cr0(u32 cr0) {
  asm volatile("movl %0, %%cr0" : : "r"(cr0));
}


static inline void set_idt(GateDesc *idt, int size) {
  volatile static u16 data[3];
  data[0] = size - 1;
  data[1] = (u32)idt;
  data[2] = (u32)idt >> 16;
  asm volatile("lidt (%0)" : : "r"(data));
}

static inline void set_gdt(SegDesc *gdt, int size) {
  volatile static u16 data[3];
  data[0] = size - 1;
  data[1] = (u32)gdt;
  data[2] = (u32)gdt >> 16;
  asm volatile("lgdt (%0)" : : "r"(data));
}

static inline void set_tr(int selector) {
  asm volatile("ltr %0" : : "r"((u16)selector));
}

static inline u32 get_cr2() {
  volatile u32 val;
  asm volatile("movl %%cr2, %0" : "=r"(val));
  return val;
}

static inline void set_cr3(void *pdir) {
  asm volatile("movl %0, %%cr3" : : "r"(pdir));
}


#endif

#endif

