#!/usr/bin/python

import sys, re, subprocess
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

