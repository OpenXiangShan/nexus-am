void _putc(char ch){
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, ch);
}

void _panic(int code){
    if(code != 0)
        printk("Panic (%d)\n",code);
    else
        printk("Hit normal exit.\n");
    asm volatile("cli;hlt");
}
