.globl intr
.type intr, @function
.globl Dismiss
.type Dismiss, @function
.globl do_irq
.type do_irq, @function
.globl vec0
.type vec0, @function
.globl vec1
.type vec1, @function
.globl vec2
.type vec2, @function
.globl vec3
.type vec3, @function
.globl vec4
.type vec4, @function
.globl vec5
.type vec5, @function
.globl vec6
.type vec6, @function
.globl vec7
.type vec7, @function
.set noat

intr:
	mfc0	$k0, $13
	mfc0	$k1, $12
	andi	$k0, $k0, 0x0000ff00
	and	$k0, $k0, $k1
	beq	$k0, $zero, Dismiss
	clz	$k0, $k0
	xori	$k0, $k0, 0x17
	la	$k1, do_irq
	jr	$k1
	nop

Dismiss:
	j	Dismiss

do_irq:
	sw 		$1, 0x4($sp)
	sw 		$2, 0x8($sp)
	sw 		$3, 0xc($sp)
	sw 		$4, 0x10($sp)
	sw 		$5, 0x14($sp)
	sw 		$6, 0x18($sp)
	sw 		$7, 0x1c($sp)
	sw 		$8, 0x20($sp)
	sw 		$9, 0x24($sp)
	sw 		$10, 0x28($sp)
	sw 		$11, 0x2c($sp)
	sw 		$12, 0x30($sp)
	sw 		$13, 0x34($sp)
	sw 		$14, 0x38($sp)
	sw 		$15, 0x3c($sp)
	sw 		$16, 0x40($sp)
	sw 		$17, 0x44($sp)
	sw 		$18, 0x48($sp)
	sw 		$19, 0x4c($sp)
	sw 		$20, 0x50($sp)
	sw 		$21, 0x54($sp)
	sw 		$22, 0x58($sp)
	sw 		$23, 0x5c($sp)
	sw 		$24, 0x60($sp)
	sw 		$25, 0x64($sp)
	sw 		$26, 0x68($sp)
	sw 		$27, 0x6c($sp)
	sw 		$28, 0x70($sp)
	sw 		$29, 0x74($sp)
	sw 		$30, 0x78($sp)
	sw 		$31, 0x7c($sp)

	li		$t0, 0x80
	beq		$t0, $k0, vec5
	nop

	li		$t0, 0x40
	beq		$t0, $k0, vec4
	nop

	li		$t0, 0x20
	beq		$t0, $k0, vec3
	nop

	li		$t0, 0x10
	beq		$t0, $k0, vec2
	nop

	li		$t0, 0x8
	beq		$t0, $k0, vec1
	nop

	li		$t0, 0x4
	beq		$t0, $k0, vec0
	nop

	li		$t0, 0x2
	beq		$t0, $k0, vec6
	nop

	li		$t0, 0x1
	beq		$t0, $k0, vec7
	nop

	lw 		$1, 0x4($sp)
	lw 		$2, 0x8($sp)
	lw 		$3, 0xc($sp)
	lw 		$4, 0x10($sp)
	lw 		$5, 0x14($sp)
	lw 		$6, 0x18($sp)
	lw 		$7, 0x1c($sp)
	lw 		$8, 0x20($sp)
	lw 		$9, 0x24($sp)
	lw 		$10, 0x28($sp)
	lw 		$11, 0x2c($sp)
	lw 		$12, 0x30($sp)
	lw 		$13, 0x34($sp)
	lw 		$14, 0x38($sp)
	lw 		$15, 0x3c($sp)
	lw 		$16, 0x40($sp)
	lw 		$17, 0x44($sp)
	lw 		$18, 0x48($sp)
	lw 		$19, 0x4c($sp)
	lw 		$20, 0x50($sp)
	lw 		$21, 0x54($sp)
	lw 		$22, 0x58($sp)
	lw 		$23, 0x5c($sp)
	lw 		$24, 0x60($sp)
	lw 		$25, 0x64($sp)
	lw 		$26, 0x68($sp)
	lw 		$27, 0x6c($sp)
	lw 		$28, 0x70($sp)
	lw 		$29, 0x74($sp)
	lw 		$30, 0x78($sp)
	lw 		$31, 0x7c($sp)

	eret

vec0:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec0
	j		irq_handle
	nop
	jr		$31
	nop

vec1:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec1
	j		irq_handle
	nop
	jr		$31
	nop

vec2:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec2
	j		irq_handle
	nop
	jr		$31
	nop

vec3:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec3
	j		irq_handle
	nop
	jr		$31
	nop

vec4:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec4
	j		irq_handle
	nop
	jr		$31
	nop

vec5:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec5
	j		irq_handle
	nop
	jr		$31
	nop

vec6:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec6
	j		irq_handle
	nop
	jr		$31
	nop

vec7:
	lw		$4, 0x4($sp)
	add		$5, $k0, $zero
	la		$k1, vec7
	j		irq_handle
	nop
	jr		$31
	nop
