# Generate riscv-crypto assembly code(sm4 instructions is machine code)
# Need to modify the file($AM_HOME/am/arch/isa/riscv64.mk) as follow: 
# -CROSS_COMPILE := riscv64-linux-gnu-
# -COMMON_FLAGS  := -fno-pic -march=rv64g -mcmodel=medany
# +CROSS_COMPILE := /bigdata/zfw/bk-tools/bin/riscv64-unknown-linux-gnu-
# +COMMON_FLAGS  := -fno-pic -march=rv64gc_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt -mcmodel=medany
#
# Run "python3 randtest.py" to generate random test
# Run "make ARCH=riscv64-noop" to compile

import random
import os

three_reg = ['aes64ds','aes64dsm', 'aes64es','aes64esm', 'aes64ks2', 'xperm.b', 'xperm.n', 'mul', 'div']

two_reg = ['aes64im', 'sm3p0', 'sm3p1',
           'sha512sig0','sha512sig1', 'sha512sum0', 'sha512sum1',
           'sha256sig0','sha256sig1', 'sha256sum0', 'sha256sum1']

sm4 = ['sm4ed', 'sm4ks'] # bs -> [0,3]
three_reg_i = ['aes64ks1i'] #imm ->[0, 10]

instructions = three_reg + three_reg_i + two_reg + sm4

out = 'random.S'
fout = open(out, 'w')

fout.write('#include "model_test.h"\n')
fout.write('#include "arch_test.h"\n')
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
    rd = str(random.randrange(32))
    rs1 = str(random.randrange(32))

    if instr in sm4:
        mask = (3 << 28) + (3 << 4) + 3
        isks = random.randrange(2) << 26
        bs_code  = random.randrange(4) << 30
        rd_code  = random.randrange(32) << 7
        rs1_code = random.randrange(32) << 15
        rs2_code = random.randrange(32) << 20
        code = str(hex(mask + bs_code + rd_code + rs1_code + rs2_code + isks))
   
        fout.write('.int ' + code + '\n')
    else: 
        if instr in three_reg:
            rs2 = str(random.randrange(32))
            if instr == 'div':
                rand_imm = hex(random.randrange(2, 2 ** 10))
                fout.write('addi x' + rs1 + ', x' + rs1 + ', ' + rand_imm + '\n')
                fout.write('addi x' + rs2 + ', x' + rs2 + ', ' + rand_imm + '\n')	
            fout.write(instr + ' x' + rd + ', x' + rs1 + ', x' + rs2 + '\n')
        elif instr in three_reg_i:
            rnum = str(random.randrange(10))
            fout.write(instr + ' x' + rd + ', x' + rs1 + ', 0x' + rnum + '\n')
        else:
            fout.write(instr + ' x' + rd + ', x' + rs1 + '\n')

fout.write('    mv a0, x0\n')
fout.write('    .int 0x0000006b\n')
fout.close()
