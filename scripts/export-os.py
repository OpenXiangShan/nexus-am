#!/usr/bin/env python3
from pathlib import Path
import re, shutil

AM_HOME = (Path(__file__) / '../..').resolve()

WHITE_LIST = [
  r'.gitignore',
  r'x86-qemu',
  r'native',
  r'^/*$',
  r'^/am/*$',
  r'^/tests/',
  r'^/apps/',
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
  r'-navy',
  r'.git',
  r'nesemu2',
  r'README.md',
  r'.DS_Store',
  r'klib/src/stdio.c',
  r'klib/src/stdlib.c',
  r'klib/src/string.c',
]


def convert(r):
  return r.replace('.', r'\.').replace('*', r'[^/]*')

def list_filter(path, xs):
  for x in xs:
    if re.search(convert(x), path):
      return True
  return False

for abspath in AM_HOME.rglob('*'):
  if abspath.is_file():
    path = abspath.relative_to(AM_HOME)
    path_str = '/' + str(path)
    white = list_filter(path_str, WHITE_LIST)
    black = list_filter(path_str, BLACK_LIST)
    if white and not black:
      print(path)

export_dir = AM_HOME / 'scripts' / 'export'
print(export_dir)
shutil.rmtree(export_dir)