target remote localhost:1234
layout asm
layout regs
set pagination off
set confirm off
file build/amtest-x86_64-qemu.o
b main
c
