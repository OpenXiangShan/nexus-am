#ifndef __AMTRACE_H__
#define __AMTRACE_H__

// payload of function calls
typedef struct _CallArgs {
  uintptr_t a0, a1, a2, a3;
} _CallArgs;

typedef union _Payload {
  _CallArgs args;
  uintptr_t retval;
} _CallPayload;

struct _TraceEvent {
  uint16_t cpu;
  uint16_t type;
  uint32_t time;
  uintptr_t ref;
  char payload[];
} __attribute__((packed));
typedef struct _TraceEvent _TraceEvent;

// ========================= trace components ========================

#define _TRACE_IOE     0x00000001
#define _TRACE_ASYE    0x00000002
#define _TRACE_PTE     0x00000004

// =========================== trace points ==========================

#define _TRACE_CALL    0x00010000
#define _TRACE_RET     0x00020000
#define _TRACE_FUNC    (_TRACE_CALL | _TRACE_RET)

#define _TRACE_ALL     0xffffffff

void _trace_on(uint32_t flags);
void _trace_off(uint32_t flags);

#endif
