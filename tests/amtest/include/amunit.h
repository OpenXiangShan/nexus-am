#ifndef __AMUNIT_H__
#define __AMUNIT_H__

#include <am.h>
#include <klib.h>

#define IOE ({ _ioe_init();  })
#define CTE(handler) ({ _cte_init(handler); })
#define VME(f1, f2) ({ _vme_init(f1, f2); })
#define MPE ({ _mpe_init(entry); })

extern const char *name;
extern void (*entry)();

#define CASE(id, name_, entry_, ...) \
  case id: { name = name_; entry = entry_; __VA_ARGS__; entry(); break; }

#endif