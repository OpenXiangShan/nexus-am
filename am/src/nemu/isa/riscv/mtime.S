# machine-mode timer interrupt handler from xv6 code

.globl __am_timervec
.align 4
__am_timervec:
        csrrw a0, mscratch, a0
        sd a1, 16(a0)
        sd a2, 24(a0)
        sd a3, 32(a0)

        # read mcause
        csrr a1, mcause
        # test whether it's an illegal instruction
        li a2, 2
        beq a1, a2, is_illegal
        # test whether it's a timer interrupt
        li a2, 1
        slli a2, a2, 63
        addi a2, a2, 7
        bne a1, a2, is_ext

is_timer:
        # schedule the next timer interrupt
        # by adding interval to mtimecmp.
        ld a1, 0(a0) # CLINT_MTIMECMP(hart)
        ld a2, 8(a0) # interval
        ld a3, 0(a1)
        add a3, a3, a2
        sd a3, 0(a1)

        # raise a supervisor timer interrupt.
	li a3, 32
        csrw mip, a3
        j end_of_intr

is_ext:
        # disable machine mode external interrupt (mie.meie)
        li a3, 0x800
        csrc mie, a3
        # raise a supervisor external interrupt
        li a3, 0x200
        csrs mip, a3

        j end_of_intr

is_illegal:
        # for now, we assume that when there's an illegal instruction exception,
        # the supervisor tries to write mie (enable mie.meie).
        li a3, 0x800
        csrs mie, a3
        # clear the supervisor external and timer interrupt
        li a3, 0x220
        csrc mip, a3
        csrr a3, mepc
        addi a3, a3, 4
        csrw mepc, a3

end_of_intr:
        ld a3, 32(a0)
        ld a2, 24(a0)
        ld a1, 16(a0)
        csrrw a0, mscratch, a0

        mret
