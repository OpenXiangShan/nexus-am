#ifndef TRAP_H
#define TRAP_H

static __attribute__((noinline)) int good() { 
    char* vga = (char*)0xc0000000 + 100*210 + 80;
    *vga++ = 'G';
    *vga++ = 'O';
    *vga++ = 'O';
    *vga++ = 'D';
    while (1);
    return 0; 
}
static __attribute__((noinline)) int bad() { 
    char* vga = (char*)0xc0000000 + 100*210 + 80;
    *vga++ = 'B';
    *vga++ = 'A';
    *vga++ = 'D';
    while (1); 
    return -1;
}

#define ASSERT(x) do { if (!(x)) bad(); } while (0)

#define set_sp asm volatile("li $sp, 16000");

#endif  // TRAP_H
