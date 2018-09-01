#ifndef __AMTRACE_H__
#define __AMTRACE_H__

#ifdef __cplusplus
extern "C" {
#endif

// payload of function calls
typedef struct _CallArgs {
  uintptr_t a0, a1, a2, a3;
} _CallArgs;

typedef union _Payload {
  _CallArgs args;
  uintptr_t retval;
} _CallPayload;

struct _TraceEvent {
  uint16_t cpu;   // cpu id (0 based)
  uint16_t type;  // _TRACE_ ## { CALL | RET }
  uint32_t time;  // time stamp
  uintptr_t ref;  // a pointer
  char payload[]; // _Payload: CALL -> .args
                  //            RET -> .retval
} __attribute__((packed));
typedef struct _TraceEvent _TraceEvent;

// ========================= trace components ========================

#define _TRACE_IOE     0x00010000
#define _TRACE_CTE     0x00020000
#define _TRACE_VME     0x00040000

// =========================== trace points ==========================

#define _TRACE_CALL    0x00000001
#define _TRACE_RET     0x00000002
#define _TRACE_FUNC    (_TRACE_CALL | _TRACE_RET)

#define _TRACE_ALL     0xffffffff

void _trace_on(uint32_t flags);
void _trace_off(uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif
