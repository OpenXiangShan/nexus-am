#include <am.h>
#include <amdev.h>
#include <nutshell.h>
#include <klib.h>

static unsigned long boot_time = 0;
static inline uint32_t read_time(void) {
  return ind(RTC_ADDR) / 1000;  // unit: ms
}

size_t __am_timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
      uptime->hi = 0;
      uptime->lo = read_time() - boot_time;
      return sizeof(_DEV_TIMER_UPTIME_t);
    }
    case _DEVREG_TIMER_DATE: {
      _DEV_TIMER_DATE_t *rtc = (_DEV_TIMER_DATE_t *)buf;
      rtc->second = 0;
      rtc->minute = 0;
      rtc->hour   = 0;
      rtc->day    = 0;
      rtc->month  = 0;
      rtc->year   = 2018;
      return sizeof(_DEV_TIMER_DATE_t);
    }
  }
  return 0;
}

void __am_timer_init() {
  boot_time = read_time();
}
