#include <am.h>

extern char _end;

_Area _heap = {
  .start = &_end,
  .end   = &_end,
};

_Screen _screen = {
  .width  = 0,
  .height = 0,
};

