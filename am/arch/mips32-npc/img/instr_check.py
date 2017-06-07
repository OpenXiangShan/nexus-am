import re, sys, subprocess
from subprocess import PIPE

def execute(commands):
  p = subprocess.Popen(commands, stdout=PIPE, stderr=PIPE)
  (out, err) = p.communicate()
  if p.returncode != 0:
    raise Exception("Execute {0} fail".format(' '.join(commands)))
  return out


def get_instr(fname):
  INST = re.compile('^([0-9a-f]+):\t([0-9a-f]+)\s+([a-z]+[0-9]*).*$')
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
      insts.add( MAP.get(inst, inst).upper() )
  return sorted(list(insts))

insts = get_instr(sys.argv[1])

print "There are {0} instructions:".format(len(insts))
print "  {0}".format(' '.join(insts))

