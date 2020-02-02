#!/usr/bin/env python3
from pathlib import Path
import re, shutil

AM_HOME    = (Path(__file__) / '../..').resolve()
EXPORT_DIR = AM_HOME / 'tools' / 'export'

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
  r'.o$',
  r'.a$',
  r'.d$',
  r'mbr$',
]


def convert(r):
  return r.replace('.', r'\.').replace('*', r'[^/]*')

def list_filter(path, xs):
  for x in xs:
    if re.search(convert(x), path):
      return True
  return False

def files():
  for abspath in AM_HOME.rglob('*'):
    if abspath.is_file():
      path = abspath.relative_to(AM_HOME)
      path_str = '/' + str(path)
      white = list_filter(path_str, WHITE_LIST)
      black = list_filter(path_str, BLACK_LIST)
      if white and not black:
        yield abspath, path
  
try:
  shutil.rmtree(EXPORT_DIR)
except:
  pass

for abspath, relpath in files():
  src = abspath
  dst = EXPORT_DIR / relpath
  dst.parent.mkdir(parents=True, exist_ok=True)
  shutil.copyfile(src, dst)
