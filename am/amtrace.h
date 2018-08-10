#ifndef __AMTRACE_H__
#define __AMTRACE_H__

struct _Trace {
  uint8_t cpu;
  uint8_t type;
  uint16_t length;
  char payload[];
} __attribute__((packed));
typedef struct _Trace _Trace;

// ========================= trace components ========================

#define _TRACE_IOE     0x00000001
#define _TRACE_ASYE    0x00000002
#define _TRACE_PTE     0x00000004
#define _TRACE_MPE     0x00000008

// =========================== trace points ==========================

#define _TRACE_CALL    0x00010000
#define _TRACE_RET     0x00020000
#define _TRACE_FUNC    (_TRACE_CALL | _TRACE_RET)
#define _TRACE_ALL     0xffffffff

void _trace_on(uint32_t flags);
void _trace_off(uint32_t flags);

#endif
