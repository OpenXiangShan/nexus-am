#include <am.h>
#include <benchlib.h>
#include <stdarg.h>

static char *utoa(uint value, char *buf, int base) {
  static char digits[] = "0123456789abcdef";
  *(buf += 34) = 0;
  do {
    *(--buf) = digits[value % base];
    value /= base;
  } while (value > 0);
  return buf;
}

static char *itoa(int value, char *buf, int base) {
  int negative = 0;
  if (value < 0) {
    negative = 1;
    value = -value;
  }
  buf = utoa(value, buf + 1, base);
  if (negative) {
    *(--buf) = '-';
  }
  return buf;
}

void printk(const char *fmt, ...) {
  va_list ap;
  char buf[50], *str;

  va_start(ap, fmt);

  for (; *fmt; fmt ++) {
    if (*fmt == '%') {
      char type = *(++ fmt);
      str = 0;
      switch (type) {
        case '%': _putc('%');
              break;
        case 'c': _putc((char)va_arg(ap, int));
              break;
        case 'd': str = itoa(va_arg(ap, int), buf, 10);
              break;
        case 'u': str = utoa(va_arg(ap, uint), buf, 10);
              break;
        case 'x': 
        case 'X': str = utoa(va_arg(ap, uint), buf, 16);
              break;
        case 's': str = va_arg(ap, char*);
              break;
        default:  _putc('%');
                  _putc(type);
      }
      while (str && *str) {
        _putc(*(str ++));
      }
    } else {
      _putc(*fmt);
    }
  }

  va_end(ap);
}

