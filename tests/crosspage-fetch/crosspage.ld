SECTIONS {
  .crosspage_test 0x82fffff8 : ALIGN(2) {
    KEEP(*(.crosspage_test))
  }
}
INSERT AFTER .bss;
