#!/usr/bin/env python3

from utils import export

WHITE_LIST = [
  r'.gitignore',
  r'x86-qemu',
  r'x86_64-qemu',
  r'native',
  r'^/*$',
  r'^/am/*$',
  r'^/share/',
  r'^/tests/amtest/',
  r'^/tests/klibtest/',
  r'^/apps/coremark/',
  r'^/apps/dhrystone/',
  r'^/apps/fceux/',
  r'^/apps/litenes/',
  r'^/apps/microbench/',
  r'^/libs/',
  r'/am/src/x86/qemu',
  r'/am/include/x86.h',
  r'am/arch/native.mk',
  r'/am/arch/isa/x86.mk',
  r'/am/arch/isa/x86_64.mk',
  r'am/include/arch/native.h',
]

BLACK_LIST = [
  r'/build/',
  r'/export/',
  r'/tools/',
  r'/.git/',
  r'-navy',
  r'README.md',
  r'.DS_Store',
  r'games/nes/gen/',
  r'tests/cputest/',
  r'klib/src/stdio.c',
  r'klib/src/stdlib.c',
  r'klib/src/string.c',
  r'.o$',
  r'.a$',
  r'.d$',
  r'mbr$',
]

export(WHITE_LIST, BLACK_LIST)
