#include <klib.h>

#define MAX_INT 0x7fffffff
#define MIN_INT 0x80000000
#define MAX_UINT 0xffffffff

#define FLAGNUM 4

static char src[1024];
static char dst[1024];

static void reset_src(){
  for(int i = 0; i < 1024; i ++)
    src[i] = 0;
}

static void check_src(char* s){
  assert( 0 == strcmp( s, src));
}


void printk_test() {
  // a string generator
  // thorough testing of all combinations
  // use assert() to check correctness

  //打印所有可显示ascii字符----c
    printk(" !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n");
  for(int i = 0; i + ' ' <= '~'; i ++){
    printk("%c", i + ' ');
    sprintf(src + i, "%c", i + ' ');
    assert(src[i] ==i + ' ');
  }
  //%s
  printk("\n%s\n", src);
  reset_src();
  sprintf(dst,"%s", src);
  assert(0 == strcmp( dst, src));

  //打印最大最小值0和各种随机值----d u x X p 
  //d u
  reset_src();
  printk("2147483647,-2147483648,0,102030,-1412505855,4294967295,0\n");
  int t=printk("%d,%d,%d,%d,%d,%u,%u\n", MAX_INT, MIN_INT, 0, 102030, -1412505855, MAX_UINT, 0);
  sprintf(src,"%d,%d,%d,%d,%d,%u,%u\n", MAX_INT, MIN_INT, 0, 102030, -1412505855, MAX_UINT, 0);
  assert(t==strlen("2147483647,-2147483648,0,102030,-1412505855,4294967295,0\n"));
  //printk("%d\n",t);
  check_src("2147483647,-2147483648,0,102030,-1412505855,4294967295,0\n");

  //x
  reset_src();
  printk("0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
  t=printk("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
  sprintf(src,"%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
  //printk("%d\n",strlen("0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
  assert(t==strlen("0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n"));
  check_src("0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
  
  //p
  reset_src();
  printk("0x0, 0xffffffff, 0x80000000, 0xabcdef01, 0xffff8000, 0x18e8e\n");
  printk("%p, %p, %p, %p, %p, %p\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
  sprintf(src,"%p, %p, %p, %p, %p, %p\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
  check_src("0x0, 0xffffffff, 0x80000000, 0xabcdef01, 0xffff8000, 0x18e8e\n");
  // flags - 0 ' ' +
  char stdstr[1024]="%  d\n";
  char flag[FLAGNUM]={' ','0','-','+'};
  //int width=0;
  //int printnum=102030;
  char* src_p=src;
  reset_src();
  for(int i=0;i<FLAGNUM;i++){
    stdstr[1]=flag[i];
    for(int j=5;j<9;j++){
      stdstr[2]='0'+j;
      printk(stdstr,102030);
      sprintf(src_p,stdstr,102030);
      src_p+=j;
      if(j==5)src_p++;
      if(flag[i]=='+')src_p++;
    }
  }
  printk("%s",src);
  check_src("102030102030 102030  102030102030102030010203000102030102030102030102030 102030  +102030+102030 +102030  +102030\n");


  
  /*这里往后是flag的测试
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
  */
}
