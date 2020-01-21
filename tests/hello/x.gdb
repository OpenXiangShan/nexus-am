target remote localhost:1234
layout asm
layout regs
set pagination off
set confirm off
add-symbol-file build/hello-x86_64-qemu.o 0x100000
b *0x100200
c
