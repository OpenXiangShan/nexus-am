.extern irq_handle
.extern _halt
.extern _stack_pointer
.set noat

.globl _trap_entry
.type _trapPentry function

_trap_entry:
# pusha, save all user registers
    lui		$k0, %hi(_stack_pointer)
    ori		$k0, %lo(_stack_pointer) // set kern stack
	addiu	$k0, $k0,-184
	sw 		$at, 0x4($k0)
	sw 		$v0, 0x8($k0)
	sw 		$v1, 0xc($k0)
	sw 		$a0, 0x10($k0)
	sw 		$a1, 0x14($k0)
	sw 		$a2, 0x18($k0)
	sw 		$a3, 0x1c($k0)
	sw 		$t0, 0x20($k0)
	sw 		$t1, 0x24($k0)
	sw 		$t2, 0x28($k0)
	sw 		$t3, 0x2c($k0)
	sw 		$t4, 0x30($k0)
	sw 		$t5, 0x34($k0)
	sw 		$t6, 0x38($k0)
	sw 		$t7, 0x3c($k0)
	sw 		$t8, 0x40($k0)
	sw 		$t9, 0x44($k0)
	sw 		$s0, 0x48($k0)
	sw 		$s1, 0x4c($k0)
	sw 		$s2, 0x50($k0)
	sw 		$s3, 0x54($k0)
	sw 		$s4, 0x58($k0)
	sw 		$s5, 0x5c($k0)
	sw 		$s6, 0x60($k0)
	sw 		$s7, 0x64($k0)
	sw 		$k0, 0x68($k0)
	sw 		$k1, 0x6c($k0)
	sw 		$gp, 0x70($k0)
	sw		$sp, 0x74($k0)
	sw 		$fp, 0x78($k0)
	sw 		$ra, 0x7c($k0)

	addu	$sp, $k0, $0

	nop
	nop
	mfc0	$k0, $14
	nop
	nop
	sw		$k0, 0x80($sp)
	nop
	nop
	mfc0	$k0, $13
	nop
	nop
	sw		$k0, 0x84($sp)
	nop
	nop
	mfc0	$k0, $12
	nop
	nop
	sw		$k0, 0x88($sp)
	nop
	nop
	mfc0	$k0, $8
	nop
	nop
	sw		$k0, 0x8c($sp)
	nop
	nop
	mfc0	$k0, $7
	nop
	nop
	sw		$k0, 0x90($sp)
# give trapframe pointer to a0
	addiu   $a0, $sp, 0x4
# jmp to irq_handle
	jal		irq_handle
	nop

# eret by irq_handle, should not reach here
	addiu	$a0, $0, 1
	jal		_halt
