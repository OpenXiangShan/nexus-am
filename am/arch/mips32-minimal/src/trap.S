.globl _trap_entry
.type _trap_entry function
_trap_entry:
  j exception_handle
