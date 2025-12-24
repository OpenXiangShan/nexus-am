#ifndef __AM_NMI_H__
#define __AM_NMI_H__

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration to avoid including am.h
typedef struct _Context _Context;
typedef struct _Event _Event;

// NMI initialization
int _nmi_init(_Context *(*handler)(_Event ev, _Context *ctx));

// Register custom NMI handler
void nmi_handler_reg(_Context*(*handler)(_Event, _Context*));

// NMI CSR definitions (RISC-V Resumable NMI Extension)
#define CSR_MNSCRATCH 0x740  // Resumable NMI scratch register
#define CSR_MNEPC     0x741  // Resumable NMI program counter
#define CSR_MNCAUSE   0x742  // Resumable NMI cause
#define CSR_MNSTATUS  0x744  // Resumable NMI status

// Note: NMI shares mtvec with M-mode, no separate mnvec

// NMI cause codes (hardware-specific, adjust as needed)
#define NMI_CAUSE_BUS_ERROR  0x1000  // Example: BEU NMI

#ifdef __cplusplus
}
#endif

#endif
