.text
.balign 8

.global h264refhotloop
.global h264refhotloop_vec
# arguments a0:source
# arguments a1:dest
# arguments a2:counter

# mv to the following registers
# a1 is the load source addr
# a2 is the counter
# a5 is the store addr
h264refhotloop:
    mv a5, a1
    mv a1, a0
    j 3f
1:
    ld a6, 16(a1)
    addi a3, a5, 24
    sd a4, 0(a0)
    ld a4, 24(a1)
    addi a0, a5, 32
    sd a6, 0(a3)
    ld a6, 32(a1)
    addi a3, a5, 40
    sd a4, 0(a0)
    ld a4, 40(a1)
    addi a0, a5, 48
    sd a6, 0(a3)
    ld a6, 48(a1)
    addi a3, a5, 56
    sd a4, 0(a0)
    ld a4, 56(a1)
    addi a5, a5, 64
    sd a6, 0(a3)
    addi a2, a2, -8
    mv a0, a5
    addi a1, a1, 64
    beqz a2, 2f
3:
    ld a6, 0(a1)
    addi a3, a5, 8
    sd a4, 0(a0)
    ld a4, 8(a1)
    addi a0, a5, 16
    sd a6, 0(a3)
    j 1b
2:
    ret


h264refhotloop_vec:
    # a0: src pointer
    # a1: dst pointer
    # a2: number of 8-byte (64-bit) elements to process

    # Set vector configuration:
    # e64: 64-bit element width (SEW=64)
    # m8: vector register group multiplier (LMUL=8)
    # ta, ma: tail agnostic, mask agnostic
    # t0 (vl) is the number of 64-bit elements processed in this iteration
    vsetvli t0, a2, e64, m8, ta, ma
    # loop expand 2 times (t0 = t0 * 2)
    # slli t0, t0, 1
    # Calculate bytes processed (vl * 8 bytes/element)
    slli t1, t0, 3

1:  
    # Load vector from src
    vle64.v v1, (a0)
    # addi t2, a0, 0x40
    # vle64.v v9, (t2)

    # Decrement remaining element count
    sub a2, a2, t0

    # Increment src pointer
    add a0, a0, t1

    # Store vector to dst
    vse64.v v1, (a1)
    # addi t3, a1, 0x40
    # vse64.v v9, (t3)
    # Increment dst pointer
    add a1, a1, t1

    # Loop if there are remaining elements
    bnez a2, 1b

    ret