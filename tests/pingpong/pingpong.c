#include <am.h>
#include <amdev.h>
#include <klib.h>

#define N 5

void f(void *s) {
  int n = 0;
  while (1) {
    printf("%s", s);
    if (n++ > 10000) {
      return; // not allowd, triggers a panic
    }
  }
}

_Context *current = NULL;
_Context contexts[N];
char texts[N][2];
uint8_t stacks[N][4096];

_Context* handler(_Event ev, _Context *ctx) {
  if (!current) {
    current = contexts; // 1st interrupt
  } else {
    *current = *ctx;
    if (current++ == contexts + N) { // round-robin
      current = contexts;
    }
  }
  return current;
}

#define STACK(id) \
  (_Area) { .start = &stacks[(id)    ][0], \
            .end   = &stacks[(id) + 1][0], }

int main(){
  printf("kcontext test started.\n");
  _asye_init(handler);
  assert(!_intr_read());
  for (int i = 0; i < N; i++) {
    texts[i][0] = 'a' + i;
    contexts[i] = *_kcontext(STACK(i), f, texts[i]);
  }
  _intr_write(1);
  while (1) {
    _yield();
  }
  return 0;
}
