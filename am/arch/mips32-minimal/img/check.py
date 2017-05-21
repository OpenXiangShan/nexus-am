#!/usr/bin/python

import sys, re, subprocess
from subprocess import PIPE

def execute(commands):
  p = subprocess.Popen(commands, stdout=PIPE, stderr=PIPE)
  (out, err) = p.communicate()
  return out


def parse(fname):
  INST = re.compile('^([0-9a-f]+):\t([0-9a-f]+)\s+([a-z]+).*$')
  insts = set()
  for line in execute(["mips-linux-gnu-objdump", "-d", fname]).split('\n'):
    m = INST.match(line.strip())
    if m: insts.add(m.group(3))
  return insts

binary = sys.argv[1]
insts = parse(binary)
print "There are {0} instructions  ".format(len(insts))
for i in insts:
  print i.upper(),
print

