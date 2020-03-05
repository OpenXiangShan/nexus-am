#include <stdint.h>
#include <limits.h>
#include <klib.h>
#include <klib-macros.h>

#ifdef __LP64__
#define TEST_LONGLONG
#endif

#define PRINTABLE_CH " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
#define STRLEN(const_str) (sizeof(const_str) - 1) // sizeof counts the null byte

#define ARRAY_DEF(type, name, smax, smin, umax) \
  static const type name [] = {0, smax / 17, smax, smin, smin + 1, umax / 17, smin / 17, umax}

ARRAY_DEF(char, char_, SCHAR_MAX, SCHAR_MIN, UCHAR_MAX);
ARRAY_DEF(short, short_, SHRT_MAX, SHRT_MIN, USHRT_MAX);
ARRAY_DEF(int, int_, INT_MAX, INT_MIN, UINT_MAX);
ARRAY_DEF(long, long_, LONG_MAX, LONG_MIN, ULONG_MAX);  // wordsize-dependent
#ifdef TEST_LONGLONG
ARRAY_DEF(long long, longlong_, LLONG_MAX, LLONG_MIN, ULLONG_MAX);
#endif
ARRAY_DEF(uintptr_t, ptr, INTPTR_MAX, INTPTR_MIN, UINTPTR_MAX);// wordsize-dependent

static_assert(LENGTH(int_) == 8);
#define TEST_FORMAT(conv, array) \
  conv " "  conv " "  conv " "  conv " "  conv " "  conv " "  conv " "  conv, \
  array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]

static char buf[1024];
static char *p_buf = NULL;

static void reset_buf() {
  for (int i = 0; i < sizeof(buf); i ++)
    buf[i] = 0;
  p_buf = buf;
}

static void check_buf(const char *ref) {
  if (0 != strcmp(ref, buf)) {
    putstr("right: \"");
    putstr(ref);
    putstr("\"\n");
    putstr("wrong: \"");
    putstr(buf);
    putstr("\"\n");

    putstr("Test fail!\n");
    _halt(1);
  }
}

static void test(const char *name, void (*fn)(void)) {
  putstr("Testing '");
  putstr(name);
  putstr("'...\n");

  reset_buf();
  fn();
}

static void test_c() {
  for(int i = 0; i < STRLEN(PRINTABLE_CH); i ++) {
    int ret = sprintf(p_buf, "%c", PRINTABLE_CH[i]);
    assert(ret == 1);
    p_buf += ret;
  }
  check_buf(PRINTABLE_CH);
}

static void test_s() {
  int ret = sprintf(buf, "%s", PRINTABLE_CH);
  check_buf(PRINTABLE_CH);
  assert(ret == STRLEN(PRINTABLE_CH));
}

#define INTEGER_TEST_DEF(name, conv, array, ref) \
  static void name() { sprintf(p_buf, TEST_FORMAT(conv, array)); check_buf(ref); }

#define SINT32_REF "0 126322567 2147483647 -2147483648 -2147483647 252645135 -126322567 -1"
#define UINT32_REF "0 126322567 2147483647 2147483648 2147483649 252645135 4168644729 4294967295"
#define SINT64_REF "0 542551296285575047 9223372036854775807 -9223372036854775808 -9223372036854775807 1085102592571150095 -542551296285575047 -1"
#define UINT64_REF "0 542551296285575047 9223372036854775807 9223372036854775808 9223372036854775809 1085102592571150095 17904192777423976569 18446744073709551615"

#if __LP64__
#define SLONG_REF SINT64_REF
#define ULONG_REF UINT64_REF
#define UINTPTR_REF "(nil) 0x787878787878787 0x7fffffffffffffff 0x8000000000000000 0x8000000000000001 0xf0f0f0f0f0f0f0f 0xf878787878787879 0xffffffffffffffff"
#else
#define SLONG_REF SINT32_REF
#define ULONG_REF UINT32_REF
#define UINTPTR_REF "(nil) 0x7878787 0x7fffffff 0x80000000 0x80000001 0xf0f0f0f 0xf8787879 0xffffffff"
#endif

INTEGER_TEST_DEF(test_d, "%d", int_, SINT32_REF)
INTEGER_TEST_DEF(test_u, "%u", int_, UINT32_REF)
INTEGER_TEST_DEF(test_x, "%x", int_, "0 7878787 7fffffff 80000000 80000001 f0f0f0f f8787879 ffffffff")
INTEGER_TEST_DEF(test_o, "%o", int_,
    "0 741703607 17777777777 20000000000 20000000001 1703607417 37036074171 37777777777")
INTEGER_TEST_DEF(test_p, "%p", ptr, UINTPTR_REF)

INTEGER_TEST_DEF(test_schar, "%hhd", char_, "0 7 127 -128 -127 15 -7 -1")
INTEGER_TEST_DEF(test_uchar, "%hhu", char_, "0 7 127 128 129 15 249 255")
INTEGER_TEST_DEF(test_sshort, "%hd", short_, "0 1927 32767 -32768 -32767 3855 -1927 -1")
INTEGER_TEST_DEF(test_ushort, "%hu", short_, "0 1927 32767 32768 32769 3855 63609 65535")
INTEGER_TEST_DEF(test_slong, "%ld", long_, SLONG_REF)
INTEGER_TEST_DEF(test_ulong, "%lu", long_, ULONG_REF)
#ifdef TEST_LONGLONG
INTEGER_TEST_DEF(test_slonglong, "%lld", longlong_, SINT64_REF)
INTEGER_TEST_DEF(test_ulonglong, "%llu", longlong_, UINT64_REF)
#endif

static const char *full_format_ans[] = {
#include "full-format-glibc.h"
};

static void test_full_format() {
  const char *flag [] = {"", " ", "0", "+", "-" };
  const char *width [] = {"", "2", "9", "15" };
  const char *prec [] = {"", ".", ".7", ".8", ".14" };
  const char *len[] = {"hh", "h", ""};
#define CONV "duxo"

  char format[32];
  int i, j, k, m, n;
  int idx = 0;
  for (i = 0; i < LENGTH(flag); i ++) {
    for (j = 0; j < LENGTH(width); j ++) {
      for (k = 0; k < LENGTH(prec); k ++) {
        for (m = 0; m < LENGTH(len); m ++) {
          for (n = 0; n < STRLEN(CONV); n ++) {
            sprintf(format, "%%%s%s%s%s%c", flag[i], width[j], prec[k], len[m], CONV[n]);
            reset_buf();
            putstr("\tTesting format \"");
            putstr(format);
            putstr("\"\n");
            sprintf(buf, format, 0xdead12ef);
            //printf("\"%s\",\n", buf);
            check_buf(full_format_ans[idx]);

            idx ++;
          }
        }
      }
    }
  }
}

void printf_test() {
  test("conversion specifier %c", test_c);
  test("conversion specifier %s", test_s);
  test("conversion specifier %d", test_d);
  test("conversion specifier %u", test_u);
  test("conversion specifier %x", test_x);
  test("conversion specifier %o", test_o);
  test("conversion specifier %p", test_p);

  test("length modifier %hhd", test_schar);
  test("length modifier %hhu", test_uchar);
  test("length modifier %hd" , test_sshort);
  test("length modifier %hu" , test_ushort);
  test("length modifier %ld" , test_slong);
  test("length modifier %lu" , test_ulong);
#ifdef TEST_LONGLONG
  test("length modifier %lld" , test_slonglong);
  test("length modifier %llu" , test_ulonglong);
#endif

  test("full" , test_full_format);

  return;
}
