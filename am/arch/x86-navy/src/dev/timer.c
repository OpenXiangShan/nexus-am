#include <am.h>
#include <amdev.h>

uint32_t systime;
int event_thread(void);

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      while (event_thread());
      _UptimeReg *uptime = (_UptimeReg *)buf;
      uptime->hi = 0;
      uptime->lo = systime;
      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
      _RTCReg *rtc = (_RTCReg *)buf;
      rtc->second = 0;
      rtc->minute = 0;
      rtc->hour   = 0;
      rtc->day    = 0;
      rtc->month  = 0;
      rtc->year   = 1900;
      return sizeof(_RTCReg);
    }
  }
  return 0;
}
