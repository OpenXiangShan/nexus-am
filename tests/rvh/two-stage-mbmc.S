/*
 * Exercise HLV/HSV, hypervisor fences, VS-stage + G-stage translation, and
 * MBMC. After a guest access fills the final RVH TLB, an HS superpage access
 * changes the current page-table level. Reusing the guest translation must
 * restore the bitmap metadata associated with the cached translation.
 */

  .option norvc
  .section .text.init
  .align  4
  .globl  _start

  .equ G_ROOT,       0x80200000
  .equ VS_ROOT,      0x80204000
  .equ HS_ROOT,      0x80205000
  .equ HS_L1,        0x80206000
  .equ TARGET_PA,    0x80401000
  .equ BITMAP_BASE,  0x80800000
  .equ BITMAP_BYTE,  0x80810080
  .equ HS_TEST_VA,   0x40000000
  .equ HS_TEST_PA,   0x80a00000

  .equ PTE_V,        0x001
  .equ PTE_VRWXAD,   0x0cf
  .equ PTE_VRWXUAD,  0x0df
  .equ SATP_SV39,    0x8000000000000000
  .equ MSTATUS_MPP_S, 0x0000000000000800
  .equ MSTATUS_MPRV,  0x0000000000020000
  .equ MSTATUS_MPV,   0x0000008000000000

_start:
  /* Permit S/VS accesses to the whole physical address space. */
  li      t0, -1
  csrw    pmpaddr0, t0
  li      t0, 0x1f
  csrw    pmpcfg0, t0

  la      t0, machine_trap
  csrw    mtvec, t0
  csrw    medeleg, zero
  csrw    mideleg, zero

  /* G-stage: identity-map 0x80000000..0xbfffffff with a 1 GiB leaf. */
  li      t0, G_ROOT
  li      t1, 0x80000000
  srli    t1, t1, 12
  slli    t1, t1, 10
  ori     t1, t1, PTE_VRWXUAD
  sd      t1, 16(t0)              /* root[2] */

  /* VS-stage: use the same identity mapping. */
  li      t0, VS_ROOT
  li      t1, 0x80000000
  srli    t1, t1, 12
  slli    t1, t1, 10
  ori     t1, t1, PTE_VRWXAD
  sd      t1, 16(t0)              /* root[2] */

  /* HS-stage: map HS_TEST_VA with a 2 MiB leaf to perturb pt_level. */
  li      t0, HS_ROOT
  li      t1, HS_L1
  srli    t1, t1, 12
  slli    t1, t1, 10
  ori     t1, t1, PTE_V
  sd      t1, 8(t0)               /* root[1] */

  li      t0, HS_L1
  li      t1, HS_TEST_PA
  srli    t1, t1, 12
  slli    t1, t1, 10
  ori     t1, t1, PTE_VRWXAD
  sd      t1, 0(t0)               /* level-1 leaf */

  li      t0, TARGET_PA
  li      t1, 0x1122334455667788
  sd      t1, 0(t0)

  /*
   * Deny the first 4 KiB page in TARGET_PA's 2 MiB region. A correct
   * level-0 lookup checks TARGET_PA's own (clear) bit; a stale level-1 lookup
   * rounds down to this set bit and raises an access fault.
   */
  li      t0, BITMAP_BYTE
  li      t1, 1
  sb      t1, 0(t0)

  li      t0, SATP_SV39
  li      t1, HS_ROOT
  srli    t1, t1, 12
  or      t1, t1, t0
  csrw    satp, t1

  li      t1, VS_ROOT
  srli    t1, t1, 12
  or      t1, t1, t0
  csrw    vsatp, t1

  li      t1, G_ROOT
  srli    t1, t1, 12
  or      t1, t1, t0
  csrw    hgatp, t1

  /* Exercise hypervisor load/store instructions through both stages. */
  li      t0, 0x100               /* hstatus.SPVP = VS */
  csrw    hstatus, t0
  li      t0, TARGET_PA
  .word   0x6c02c373              /* hlv.d t1, (t0) */
  li      t2, 0x1122334455667788
  bne     t1, t2, fail_hlv

  li      t2, 0x8877665544332211
  .word   0x6e72c073              /* hsv.d t2, (t0) */
  .word   0x6c02c373              /* hlv.d t1, (t0) */
  bne     t1, t2, fail_hsv

  /* Restore the target and cover both H-extension translation fences. */
  li      t2, 0x1122334455667788
  sd      t2, 0(t0)
  .word   0x22000073              /* hfence.vvma zero, zero */
  .word   0x62000073              /* hfence.gvma zero, zero */

  /* mbmc.BMA = BITMAP_BASE, BME = 1, CMODE = 0. */
  li      t0, BITMAP_BASE + 4
  csrw    0xbc2, t0

  la      t0, guest_start
  csrw    mepc, t0
  li      t0, MSTATUS_MPP_S | MSTATUS_MPV
  csrw    mstatus, t0
  mret

guest_start:
  /* Fill the read translation before pt_level is perturbed. */
  li      t0, TARGET_PA
  ld      t1, 0(t0)
  li      t2, 0x1122334455667788
  bne     t1, t2, fail_first_load

  ecall

  /* The final RVH TLB should hit here and restore pt_level = 0. */
  li      t0, TARGET_PA
  ld      t1, 0(t0)
  bne     t1, t2, fail_second_load

pass:
  li      a0, 0
  .word   0x0005006b              /* nemu_trap */

fail_first_load:
  li      a0, 1
  .word   0x0005006b              /* nemu_trap */

fail_second_load:
  li      a0, 2
  .word   0x0005006b              /* nemu_trap */

fail_hlv:
  li      a0, 3
  .word   0x0005006b              /* nemu_trap */

fail_hsv:
  li      a0, 4
  .word   0x0005006b              /* nemu_trap */

fail_trap:
  addi    a0, t0, 20
  .word   0x0005006b              /* nemu_trap */

machine_trap:
  csrr    t0, mcause
  li      t1, 10                  /* ecall from VS-mode */
  bne     t0, t1, fail_trap

  /* Perform an HS 2 MiB-page load without changing any translation CSR. */
  li      t0, MSTATUS_MPP_S | MSTATUS_MPRV
  csrw    mstatus, t0
  li      t0, HS_TEST_VA
  ld      t1, 0(t0)

  csrr    t0, mepc
  addi    t0, t0, 4
  csrw    mepc, t0
  li      t0, MSTATUS_MPP_S | MSTATUS_MPV
  csrw    mstatus, t0
  mret
