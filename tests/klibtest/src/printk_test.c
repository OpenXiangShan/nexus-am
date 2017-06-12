#include <klib.h>

void printk_test() {
  // sprintf: TODO

  // a string generator
  // thorough testing of all combinations
  // use assert() to check correctness

  // printk: TODO
  
  static char src[1024]="hello";
  printk("0, -1, -2147483648, -1412505855, -32768, 102030\n");
  printk("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
  sprintf(src,"%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
  printk("%s",src);
  printk("0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
  printk("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);

  printk(" -102030\n");
  printk("%*d\n",8,-102030);
  printk("102030  x\n");
  printk("%-8dx\n",102030);
  printk("  102030x\n");
  printk("%8dx\n",102030);
  printk("4294967295\n");
  printk("%u\n",4294967295u);
  printk("ffffffff\n");
  printk("%x\n",0xffffffff);
  printk("0, -1, -2\n");
}
