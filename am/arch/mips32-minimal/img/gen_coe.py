#!/usr/bin/python

import sys, re, subprocess, binascii
from subprocess import PIPE

def execute(commands):
  p = subprocess.Popen(commands, stdout=PIPE, stderr=PIPE)
  (out, err) = p.communicate()
  return out


def parse(fname):
  # objdump result
  INST = re.compile('^([0-9a-f]+):\t([0-9a-f]+)\s+([a-z]+).*$')
  # pseudo instrutions
  MAP = {
    'beqz': 'beq',
    'bnez': 'bne',
    'negu': 'subu',
  }

  insts = set()
  for line in execute(["mips-linux-gnu-objdump", "-M", "no-aliases", "-d", fname]).split('\n'):
    m = INST.match(line.strip())
    if m:
      inst = m.group(3)
      insts.add( MAP.get(inst, inst) )
  return sorted(list(insts))

binary = sys.argv[1]
insts = parse(binary)
print "There are {0} instructions:".format(len(insts))
print "  {0}".format(" ".join([i.upper() for i in insts]))

def gen_coe(fname):
  assembly = execute(["mips-linux-gnu-objdump", "-d", fname])
  names = execute(["mips-linux-gnu-nm", fname])
  with open(fname + ".txt", "w") as f:
    f.write(assembly)
    f.write(names)
    f.close()

  bins = execute(["mips-linux-gnu-objcopy", "-O", "binary", fname, "/dev/stdout"])

  while len(bins) % 4 != 0:
    bins = bins + '\0'

  with open(fname + ".coe", 'w') as f:
    f.write('memory_initialization_radix=16;\nmemory_initialization_vector=\n')
    for i in range(0, len(bins)/4):
      ii = i * 4
      contents = binascii.b2a_hex(bins[ii + 3] + bins[ii + 2] + bins[ii + 1] + bins[ii])
      f.write( contents )
      if i != (len(bins)/4-1):
        f.write(',\n')
      else:
        f.write(';')
    f.write('\n')
    f.close()

gen_coe(binary)
exit()


