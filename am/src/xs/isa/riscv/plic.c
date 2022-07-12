#include <am.h>
#include <xs.h>

#define READ_WORD(addr)        (*((volatile uint32_t *)(addr)))
#define WRITE_WORD(addr, data) (*((volatile uint32_t *)(addr)) = (data))
#define EXTRACT_BIT(data, i)   ((data) & (0x1UL << (i)))
#define SET_BIT(data, i)       ((data) | (0x1UL << (i)))
#define CLEAR_BIT(data, i)     ((data) ^ EXTRACT_BIT(data, i))

#define INTR_REG_WIDTH         32
#define INTR_REG_ADDR(i)       ((INTR_GEN_ADDR) + ((i) << 2))
#define INTR_REG_INDEX(i)      INTR_REG_ADDR(((i) / INTR_REG_WIDTH))
#define INTR_REG_OFFSET(i)     ((i) % INTR_REG_WIDTH)

#define INTR_RANDOM_ADDR(i)    ((INTR_RANDOM) + ((i) << 2))

#define READ_INTR_REG(i)  READ_WORD(INTR_REG_ADDR(i))
#define READ_INTR(i)     EXTRACT_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i))
#define CLEAR_INTR(i)    WRITE_WORD(INTR_REG_INDEX(i), CLEAR_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i)))
#define SET_INTR(i)      WRITE_WORD(INTR_REG_INDEX(i), SET_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i)))

#define CONTEXT_M 0
#define CONTEXT_S 1
#define PLIC_PRIORITY          (PLIC_BASE_ADDR + 0x4UL)
#define PLIC_PENDING           (PLIC_BASE_ADDR + 0x1000UL)
#define PLIC_ENABLE(c)         (PLIC_BASE_ADDR + 0x2000UL + c*0x80UL)
#define PLIC_THRESHOLD(c)      (PLIC_BASE_ADDR + 0x200000UL + c*0x1000UL)
#define PLIC_CLAIM(c)          (PLIC_BASE_ADDR + 0x200004UL + c*0x1000UL)
// External interrupts start with index PLIC_EXT_INTR_OFFSET
#define PLIC_EXT_INTR_OFFSET   1

// CSR.MIE
#define MEIE 11
#define SEIE 9
#define MSIE 3
#define SSIE 1

/*
 * Driver for PLIC
 * extintr test in amtest use these functions, which may serve as a functional verification
 */

/*
 * PLIC claim read function
 * return the claim number of current context
 */
uint32_t plic_get_claim(uint32_t current_context) {
  return READ_WORD(PLIC_CLAIM(current_context));
}
/*
 * PLIC intrrupt clear function
 * the interrupt signal of given claim is cleared
 */
void plic_clear_intr(uint32_t claim) {
  CLEAR_INTR(claim - PLIC_EXT_INTR_OFFSET);
}

/*
 * PLIC claim clear function
 * write the claim back to clear
 */
void plic_clear_claim(uint32_t current_context, uint32_t claim) {
 WRITE_WORD(PLIC_CLAIM(current_context), claim);
}

/*
 * PLIC priority set function
 * intr: interrupt source number
 * priority: the priority to be set
 */
void plic_set_priority(uint32_t intr, uint32_t priority) {
  WRITE_WORD(PLIC_PRIORITY + intr * sizeof(uint32_t), priority);
}

/*
 * PLIC interrupt enable function
 * current_context: the context where this interrupt will be enabled
 * intr: interrupt source number
 */
void plic_enable(uint32_t current_context, uint32_t intr) {
  // must read first to avoid unset other sources
  uint32_t origin = READ_WORD(PLIC_ENABLE(current_context) + (intr / 32) * 4);
  origin |= 1UL << (intr % 32);
  WRITE_WORD(PLIC_ENABLE(current_context) + (intr / 32) * 4, origin);
}

void plic_disable(uint32_t current_context, uint32_t intr) {
  // must read first to avoid unset other sources
  uint32_t origin = READ_WORD(PLIC_ENABLE(current_context) + (intr / 32) * 4);
  origin ^= 1UL << (intr % 32);
  WRITE_WORD(PLIC_ENABLE(current_context) + (intr / 32) * 4, origin);
}


/*
 * PLIC interrupt disable function
 * PLIC interrupt is clusterd by 32-bit word, disable them efficiently by clearing the whole word
 * current_context: the context where interrupt will be disabled
 * intr: the interrupt source number， all interrupts that inside this word will be disabled
 */
void plic_disable_word(uint32_t current_context, uint32_t intr) {
  WRITE_WORD(PLIC_ENABLE(current_context) + intr/8, 0);
}

/*
 * PLIC threshold set function
 */
void plic_set_threshold(uint32_t current_context, uint32_t threshold) {
 WRITE_WORD(PLIC_THRESHOLD(current_context), threshold);
}

/*
 * PLIC interrupt set function
 * manually set an interrupt
 * intr: interrupt source number
 */
void plic_set_intr(uint32_t intr) {
 SET_INTR(intr - PLIC_EXT_INTR_OFFSET);
}

