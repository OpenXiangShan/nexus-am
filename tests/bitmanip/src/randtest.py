# Generate riscv-bitmanip assembly code
# Need to modify the file($AM_HOME/am/arch/isa/riscv64.mk) as follow:
# -CROSS_COMPILE := riscv64-linux-gnu-
# -COMMON_FLAGS  := -fno-pic -march=rv64g -mcmodel=medany
# +CROSS_COMPILE := /bigdata/zfw/b-tools/bin/riscv64-unknown-linux-gnu-
# +COMMON_FLAGS  := -fno-pic -march=rv64gc_zba_zbb_zbc_zbs -mcmodel=medany
#
# Run "python3 randtest.py" to generate random test
# Run "make ARCH=riscv64-noop" to compile


import random
import os

three_reg = ['andn','orn','xnor',
            'rol','ror','rolw','rorw',
            'sbclr','sbinv','sbext','sbset',
            'pack', 'packw', 'packh', 
            'clmul', 'clmulr', 'clmulh', 
            'max', 'min', 'maxu', 'minu',
            'sh1add', 'sh2add', 'sh3add', 'sh1adduw', 'sh2adduw', 'sh3adduw', 'adduw'
            ]

two_reg = ['pcnt', 'pcntw','clz', 'ctz', 'clzw', 'ctzw']
special = ['gorci', 'grevi'] #['orcb', 'rev8']
three_reg_i32 = ['slliu_w', 'roriw']
three_reg_i64 = ['sbclri','sbinvi','sbexti','sbseti', 'rori']

instructions = three_reg + three_reg_i32 + three_reg_i64 + two_reg + special

out = 'random.S'
fout = open(out, 'w')

fout.write('#include "extB.S.include"\n')
fout.write('.section .text.init\n')
fout.write('.global _trm_init\n')
fout.write('_trm_init:\n')
fout.write('    j START_RAMDOM_TEST\n')
fout.write('START_RAMDOM_TEST:\n')

TEST_SIZE = 10 ** 8    

# initial
for r in range(1,32):
    rand_data = hex(random.randrange(2 ** 64))
    fout.write('li x' + str(r) + ',' + str(rand_data) + '\n')

for i in range(0, TEST_SIZE):
    instr = random.choice(instructions)
    rd = str(random.randrange(31) + 1)  # skip x0
    rs1 = str(random.randrange(32))

    fout.write(instr + ' ' + rd + ' ' + rs1)

    if instr in two_reg:
        fout.write('\n')
    elif instr in three_reg_i32:
        shamt32 = str(random.randrange(32))
        fout.write(' ' + shamt32 + '\n')
    elif instr in three_reg_i64:
        shamt64 = str(random.randrange(64))
        fout.write(' ' + shamt64 + '\n')
    elif instr == 'gorci':
        fout.write(' 7\n')
    elif instr == 'grevi':
        fout.write(' 56\n')
    else:
        rs2 = str(random.randrange(32))
        fout.write(' ' + rs2 + '\n')

fout.write('    mv a0, x0\n')
fout.write('    .int 0x0000006b\n')
fout.close()
