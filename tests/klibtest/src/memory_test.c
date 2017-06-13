#include <klib.h>

#define N 64
static u8 data[N];

static void reset() {
  for (int i = 0; i < N; i ++) {
    data[i] = i;
  }
}

static void check_equal(int st, int ed, int val) {
  for (int i = st; i < ed; i ++) {
    assert(data[i] == val);
  }
}

static void check_sequence(int st, int ed, int initval) {
  for (int i = st; i < ed; i ++) {
    assert(data[i] == initval + i - st);
  }
}

void memory_test() {
  // memset
  for (int st = 0; st < N; st ++)
    for (int ed = st + 1; ed <= N; ed ++) {
      reset();
      u8 val = (st + ed) / 2;
      memset(data + st, val, ed - st);
      check_sequence(0, st, 0);
      check_sequence(ed, N, ed);
      check_equal(st, ed, val);
    }

  // memmove memcpy
  for (int st = 0; st < N; st ++) {
    for (int ed = st + 1; ed <= N; ed ++) {
      int len = ed - st;
      for (int cp = 0; cp + len <= N; cp ++) {
        reset();
        memmove(data + st, data + cp, len);
        check_sequence(0, st, 0);
        check_sequence(st, st + len, cp);
        check_sequence(st + len, N, st + len);
      }
    }
  }

  // memcpy: memcpy是用的memmove实现所以共用一个测试即可

  // strcpy / strncpy: TODO
  for (int st = 0; st < N; st ++) {
    for (int ed = st + 1; ed <= N; ed ++) {
      int len = ed - st;
      for (int cp = 0; cp + len <= N; cp ++) {
        reset();
        if((cp > st && cp < ed)||(cp + len > st && cp + len < ed)) continue;
	strncpy((char *)data + st,(char *)data + cp,len);
        check_sequence(0, st, 0);
        check_sequence(st, st + len, cp);
        check_sequence(st + len, N, st + len);
      }
    }
  }
  /*
  for (int st = 0; st < N; st ++) {
    for (int ed = st + 1; ed <= N; ed ++) {
      int len = ed - st;
      for (int cp = 0; cp + len <= N; cp ++) {
	//这里是因为 strcpy的内部是不会处理内存重叠问题，标准库中的strcpy也会出错，所以测试就不测试内存重叠了。
        reset();
        if((cp > st && cp < ed)||(cp + len > st && cp + len < ed)) continue;
	data[cp + len]='\0';
	strcpy((char *)data + st, (char *)data + cp);
	data[cp + len]=cp + len;
        //check_sequence(0, st, 0);
        //check_sequence(st, st + len - 1, cp);
        //check_sequence(ed, N, st + len);
	//printk("%d",cp);
      }
    }
  }*/

  // strcmp / strncmp: TODO
}
