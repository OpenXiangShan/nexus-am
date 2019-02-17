#include <am.h>
#include <x86.h>
#include <amdev.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
      uptime->hi = 0;
      uptime->lo = inl(RTC_PORT) - boot_time;
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

void timer_init() {
  boot_time = inl(RTC_PORT);
}
