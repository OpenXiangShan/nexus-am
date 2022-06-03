#ifndef __AMUNIT_H__
#define __AMUNIT_H__

#include <am.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#define IOE ({ _ioe_init();  })

#define CTE(h) ({ _Context *h(_Event, _Context *); _cte_init(h); })
#define REEH(h) ({ _Context *h(_Event, _Context *); seip_handler_reg(h);})
#define RTEH(h) ({ _Context *h(_Event, _Context *); stip_handler_reg(h);})
#define RCEH(h) ({ _Context *h(_Event, _Context *); secall_handler_reg(h);})
#define NOTIMEINT() ({ extern int g_config_disable_timer; g_config_disable_timer = 1;})

#define VME(f1, f2) ({ void *f1(size_t); void f2(void *); _vme_init(f1, f2); })
#define PRE_MPE(arg) ({ _mpe_setncpu(arg); })
#define MPE(f) ({ void f(); _mpe_init(f); })

extern void (*entry)();

#define CASE(id, entry_, ...) \
  case id: { \
    void entry_(); \
    entry = entry_; \
    __VA_ARGS__; \
    entry_(); \
    break; \
  }

#endif
