# machine-mode timer interrupt handler from xv6 code

.globl __am_timervec
.align 4
__am_timervec:
        csrrw a0, mscratch, a0
        sd a1, 16(a0)
        sd a2, 24(a0)
        sd a3, 32(a0)

        # schedule the next timer interrupt
        # by adding interval to mtimecmp.
        ld a1, 0(a0) # CLINT_MTIMECMP(hart)
        ld a2, 8(a0) # interval
        ld a3, 0(a1)
        add a3, a3, a2
        sd a3, 0(a1)

        # raise a supervisor software interrupt.
        csrwi sip, 2

        ld a3, 32(a0)
        ld a2, 24(a0)
        ld a1, 16(a0)
        csrrw a0, mscratch, a0

        mret
