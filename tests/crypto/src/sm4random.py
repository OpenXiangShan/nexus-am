import random
import os

out = 'sm4random.S'
fout = open(out, 'w')

fout.write('#include "model_test.h"\n')
fout.write('#include "arch_test.h"\n')
fout.write('.section .text.init\n')
fout.write('.global _trm_init\n')
fout.write('_trm_init:\n')
fout.write('    j START_RAMDOM_TEST\n')
fout.write('START_RAMDOM_TEST:\n')

    

# initial
for r in range(1,32):
    rand_data = hex(random.randrange(2 ** 64))
    fout.write('li x' + str(r) + ',' + str(rand_data) + '\n')

for i in range(0, 10 ** 4):
    mask = (3 << 28) + (3 << 4) + 3
    isks = random.randrange(2) << 26
    bs  = random.randrange(4) << 30
    rd  = random.randrange(32) << 7
    rs1 = random.randrange(32) << 15
    rs2 = random.randrange(32) << 20
    code = str(hex(mask + bs + rd + rs1 + rs2 + isks))

    fout.write('.int ' + code + '\n')


fout.write('    mv a0, x0\n')
fout.write('    .int 0x0000006b\n')
fout.close()
