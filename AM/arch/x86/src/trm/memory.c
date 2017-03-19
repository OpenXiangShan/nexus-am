static void memory_init() {
    extern char end;
    unsigned long st, ed;
    unsigned long step = 1L<<20; // 1 MB step
    st = ed = (((unsigned long)&end) & ~(step-1)) + step;
    while (true) {
        volatile int *ptr = (int*)ed;
        *ptr = 0x5a5a5a5a;
        if (*ptr == 0x5a5a5a5a) {
            ed += step;
        } else {
            break;
        }
    }
    printk("Probed memory %d MB\n", ed >> 20);
    printk("  [%x, %x)\n", st, ed);
}
