#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"
#include <elf.h>

uint32_t expr(char *, bool *);
int get_sym(char *var);
void get_funcname(swaddr_t addr);
char *strtab;
Elf32_Sym *symtab;
int nr_symtab_entry;

#endif
