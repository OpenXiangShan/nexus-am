#!/usr/bin/env python3

import os

roms = []

for (root, dirs, files) in os.walk('rom'):
  for f in files:
    if f.endswith('.nes'):
      name = f.split('.')[0]
      if not os.path.exists(f'gen/{name}.c'):
        os.system(f'xxd -i "{root}/{f}" > "gen/{name}.c"')
      roms.append(name)

for (root, dirs, files) in os.walk('gen'):
  for f in files:
    if f.endswith('.c') and f.split('.')[0] not in roms:
      os.remove(f'{root}/{f}')

def h_file():
  for name in roms:
    yield f'extern unsigned char rom_{name}_nes[];'
  yield '''
struct rom {
  const char *name;
  void *body;
};

struct rom roms[] = {'''
  for name in roms:
    yield f'  {{ .name = "{name}", .body = rom_{name}_nes, }},'
  yield '};'

  yield f'int nroms = {len(roms)};'


with open('gen/roms.h', 'w') as fp:
  for line in h_file():
    fp.write(line)
    fp.write('\n')
