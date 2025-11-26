.text
.balign 8

.global csrrw_loop

# a0: loop count
csrrw_loop: 
1:
    csrr a4, fflags
    # flt.d a5, fs1, fa4
    addi a5, a5, 1
    csrw fflags, a4
    addi a0, a0, -1
    bnez a0, 1b
    ret