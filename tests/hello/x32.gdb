target remote localhost:1234
layout asm
layout regs
set pagination off
set confirm off
file build/hello-x86_32-qemu.o
b *0x7c00
c
