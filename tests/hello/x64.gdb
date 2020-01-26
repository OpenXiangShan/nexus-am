target remote localhost:1234
layout asm
layout regs
set pagination off
set confirm off
file build/hello-x86_64-qemu.o
b _start64
b _start
c
