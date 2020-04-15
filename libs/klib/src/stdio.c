#include "klib.h"
#include <stdarg.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef struct {
  struct {
    union { int enable; int is_right; };
    char ch;
  } pad, sign;

  char *out;
  const char *end;
  size_t nbyte;
} format_t;

static void __putch(char c, int n, format_t *f) {
  while (n --) {
    if (f->out == NULL) _putc(c);
    // do not output if we reach the given size of the buffer
    else if (f->out != f->end) { *((f->out) ++) = c; }
    f->nbyte ++;
  }
}

static void __putstr(const char *s, int len, format_t *f) {
  while (len --) __putch(*s ++, 1, f);
}

#define FORMAT_FUN_DEF(name, type, base) \
  static int name(char *revbuf, type n) { \
    char *p = revbuf; \
    *(-- p) = '\0'; \
    do { \
      *(-- p) = "0123456789abcdef"[n % base]; \
      n /= base; \
    } while (n); \
    return revbuf - p - 1;  /* exclude the null byte */ \
  }

FORMAT_FUN_DEF(format_oct32, uint32_t, 8)
FORMAT_FUN_DEF(format_dec32, uint32_t, 10)
FORMAT_FUN_DEF(format_hex32, uint32_t, 16)
FORMAT_FUN_DEF(format_oct64, uint64_t, 8)
FORMAT_FUN_DEF(format_dec64, uint64_t, 10)
FORMAT_FUN_DEF(format_hex64, uint64_t, 16)

static int format_int32(char *rev_buf, unsigned long long n, int base) {
  switch (base) {
    case 8:  return format_oct32(rev_buf, n);
    case 10: return format_dec32(rev_buf, n);
    case 16: return format_hex32(rev_buf, n);
  }
  return 0;
}

static int format_int64(char *rev_buf, unsigned long long n, int base) {
  switch (base) {
    case 8:  return format_oct64(rev_buf, n);
    case 10: return format_dec64(rev_buf, n);
    case 16: return format_hex64(rev_buf, n);
  }
  return 0;
}

enum { LEN_NONE, LEN_hh, LEN_h, LEN_l, LEN_ll };

static int vsnprintf_internal(char *out, size_t size, const char *fmt, va_list ap) {
  format_t f = {};
  f.out = out;
  f.end = (out && (size != 0) ? out + size : NULL);
  f.nbyte = 0;

  const char *pfmt = fmt;
  while (*pfmt) {
    char ch = *pfmt ++;
    if (ch != '%') {
      __putch(ch, 1, &f);
      continue;
    }

    char temp_buf[64];
    char *temp_buf_end = temp_buf + sizeof(temp_buf);
    uint32_t varguint32 = 0;
    uint64_t varguint64 = 0;
    int is_64bit = false;
#ifdef __LP64__
#define vargulong varguint64
#else
#define vargulong varguint32
#endif

    f.pad.is_right = false; f.pad.ch = ' ';
    f.sign.enable = false;
    int width = 0;
    int prec = 0;
    int len_mod = LEN_NONE;
    int is_signed = false;
    int base = 0;
    int conv_len = 0;
    int width_pad_len = 0;
    char *p_str;

next_ch:
    ch = *pfmt ++;
reswitch:
    switch (ch) {
      // flag character
      case '#': goto next_ch; // not implement
      case '-': f.pad.is_right = true; f.pad.ch = ' '; goto next_ch;
      case '0': if (!f.pad.is_right) f.pad.ch = '0'; goto next_ch;
      case ' ': if (!(f.sign.enable && f.sign.ch == '+')) {
                  f.sign.enable = true; f.sign.ch = ' ';
                }
                goto next_ch;
      case '+': f.sign.enable = true; f.sign.ch = '+'; goto next_ch;

      // field width
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
        width = ch - '0';
        while ((ch = *pfmt ++) >= '0' && ch <= '9') {
          width = width * 10 + ch - '0';
        }
        if (ch == '-') { width = -width; goto next_ch; }
        else goto reswitch;

      // presicion;
      case '.':
        while ((ch = *pfmt ++) >= '0' && ch <= '9') {
          prec = prec * 10 + ch - '0';
        }
        f.pad.ch = ' ';  // ignore '0' flag
        goto reswitch;

      // length modifier
      case 'h':
        if ((ch = *pfmt ++) == 'h') { len_mod = LEN_hh; goto next_ch; }
        else { len_mod = LEN_h; goto reswitch; }
      case 'l':
        if ((ch = *pfmt ++) == 'l') { len_mod = LEN_ll; goto next_ch; }
        else { len_mod = LEN_l; goto reswitch; }

      // conversion specifier
      case 'd': case 'i': is_signed = true; // fall through
      case 'u': base = 10; goto read_arg;
      case 'x': case 'X': base = 16; goto read_arg;
      case 'o': base = 8; goto read_arg;

read_arg:
#define CASE(len_mod, actual_type, fetch_type, varguint) \
  case len_mod: { \
    signed actual_type s = (signed actual_type)va_arg(ap, signed fetch_type); \
    is_64bit = (sizeof(varguint) == 8); \
    if (is_signed && s < 0) { \
      f.sign.ch = '-'; f.sign.enable = true; \
      /* avoid overflow when performing negation with INT_MIN */ \
      varguint = (unsigned actual_type)(-(s + 1)) + 1; \
    } else varguint = (unsigned actual_type)s; \
    break; \
  }
        switch (len_mod) {
          CASE(LEN_hh, char, int, varguint32)
          CASE(LEN_h, short, int, varguint32)
          CASE(LEN_NONE, int, int, varguint32)
          CASE(LEN_l, long, long, vargulong)
          CASE(LEN_ll, long long, long long, varguint64)
        }
print_num:;
        int digit_len = (is_64bit ? format_int64(temp_buf_end, varguint64, base) :
            format_int32(temp_buf_end, varguint32, base));
        char *p_digit = temp_buf_end - digit_len - 1;
        int sign_len = (is_signed && f.sign.enable ? 1 : 0);
        int prec_pad0_len = (prec > digit_len ? prec - digit_len : 0);
        conv_len = sign_len + prec_pad0_len + digit_len;
        width_pad_len = (width > conv_len ? width - conv_len : 0);

        if (!f.pad.is_right && f.pad.ch == ' ') { __putch(f.pad.ch, width_pad_len, &f); }
        if (sign_len) __putch(f.sign.ch, 1, &f);
        if (!f.pad.is_right && f.pad.ch == '0') { __putch(f.pad.ch, width_pad_len, &f); }
        __putch('0', prec_pad0_len, &f);
        __putstr(p_digit, digit_len, &f);
        if (f.pad.is_right) { __putch(f.pad.ch, width_pad_len, &f); }
        break;

      case 'p':
        vargulong = va_arg(ap, uintptr_t);
        if (vargulong == 0) { __putstr("(nil)", 5, &f); break; }
        __putstr("0x", 2, &f);
        base = 16;
        is_64bit = (sizeof(vargulong) == 8);
        goto print_num;

      case 'c':
        conv_len = 1;
        char c = (char)va_arg(ap, int);
        p_str = &c;
        goto print_str;

      case 's':
        p_str = va_arg(ap, char *);
        conv_len = (prec == 0 ? strlen(p_str) : prec);
print_str:
        width_pad_len = (width > conv_len ? width - conv_len : 0);
        if (!f.pad.is_right) { __putch(f.pad.ch, width_pad_len, &f); }
        __putstr(p_str, conv_len, &f);
        if (f.pad.is_right) { __putch(f.pad.ch, width_pad_len, &f); }
        break;

      case '%': __putch('%', 1, &f); break;

      default:;
    }
  }

  if (f.out != NULL) { *f.out = '\0'; }
  return f.nbyte;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf_internal(NULL, 0, fmt, ap);
  va_end(ap);
  return r;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf_internal(out, 0, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf_internal(out, 0, fmt, ap);
  va_end(ap);
  return r;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf_internal(out, n, fmt, ap);
  va_end(ap);
  return r;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return vsnprintf_internal(out, n, fmt, ap);
}

#endif
