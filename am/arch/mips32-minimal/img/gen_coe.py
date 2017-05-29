#!/usr/bin/python

import sys, re, subprocess, binascii
from subprocess import PIPE

def execute(commands):
  p = subprocess.Popen(commands, stdout=PIPE, stderr=PIPE)
  (out, err) = p.communicate()
  if p.returncode != 0:
    raise Exception("Execute {0} fail".format(' '.join(commands)))
  return out

def instr_check(fname):
  INST = re.compile('^([0-9a-f]+):\t([0-9a-f]+)\s+([a-z]+).*$')
  MAP = {
    'beqz': 'beq',
    'bnez': 'bne',
    'negu': 'subu',
  }

  insts = set()
  asm = execute(["mips-linux-gnu-objdump", "-M", "no-aliases", "-d", fname])

  with open(fname + '.txt', 'w') as fp:
    fp.write(asm)

  for line in asm.split('\n'):
    m = INST.match(line.strip())
    if m:
      inst = m.group(3)
      insts.add( MAP.get(inst, inst) )
  return sorted(list(insts))

def gen_coe(fname):
  execute(["mips-linux-gnu-objcopy", "-O", "binary", fname, fname + ".bin"])
  with open(fname + ".bin", "r") as fp:
    bins = fp.read() # read from binary
  bins += '\0' * int(execute(["size", fname]).split('\n')[1].split()[2]) # pad bss
  bins += '\0' * (4 - len(bins) % 4) # data align

  with open(fname + ".coe", 'w') as f:
    f.write('memory_initialization_radix=16;\n')
    f.write('memory_initialization_vector=\n')

    for i in range(0, len(bins), 4):
      f.write("  {0}{1}\n".format(
        binascii.b2a_hex(bins[i:i + 4][::-1]),
        [",", ";"][0 if i + 4 < len(bins) else 1] ) )
    f.close()

binary = sys.argv[1]
insts = instr_check(binary)
print "There are {0} instructions:".format(len(insts))
print "  {0}".format(" ".join([i.upper() for i in insts]))

gen_coe(binary)
