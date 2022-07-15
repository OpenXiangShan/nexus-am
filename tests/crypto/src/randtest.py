# Generate riscv-crypto assembly code(sm4 instructions is machine code)

import argparse
import random
from multiprocessing import Process, Queue

three_reg = ['aes64ds','aes64dsm', 'aes64es','aes64esm', 'aes64ks2', 'xperm.b', 'xperm.n', 'mul', 'div']

two_reg = ['aes64im', 'sm3p0', 'sm3p1',
           'sha512sig0','sha512sig1', 'sha512sum0', 'sha512sum1',
           'sha256sig0','sha256sig1', 'sha256sum0', 'sha256sum1']

sm4 = ['sm4ed', 'sm4ks'] # bs -> [0,3]
three_reg_i = ['aes64ks1i'] #imm ->[0, 10]

instructions = three_reg + three_reg_i + two_reg + sm4

special = ['gorci', 'grevi'] #['orcb', 'rev8']
three_reg_i32 = ['slliu_w', 'roriw']
three_reg_i64 = ['sbclri','sbinvi','sbexti','sbseti', 'rori']
three_reg_b = ['andn','orn','xnor',
    'rol','ror','rolw','rorw',
    'sbclr','sbinv','sbext','sbset',
    'pack', 'packw', 'packh',
    'clmul', 'clmulr', 'clmulh',
    'max', 'min', 'maxu', 'minu',
    'sh1add', 'sh2add', 'sh3add', 'sh1adduw', 'sh2adduw', 'sh3adduw', 'adduw'
]
two_reg_b = ['pcnt', 'pcntw','clz', 'ctz', 'clzw', 'ctzw']

generate_b_tests = True
if generate_b_tests:

    instructions += three_reg_b + two_reg_b + three_reg_i32 + three_reg_i64 + special

def generate_asm(out, test_size):
    asm_name = f"{out}.S"

    fout = open(asm_name, 'w')

    if generate_b_tests:
        fout.write('#include "../../bitmanip/include/extB.S.include"\n')
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

    print(f"Generate {test_size} cases to {asm_name}")

    for i in range(0, test_size):
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
        elif instr in three_reg:
            rs2 = str(random.randrange(32))
            if instr == 'div':
                rand_imm = hex(random.randrange(2, 2 ** 10))
                fout.write('addi x' + rs1 + ', x' + rs1 + ', ' + rand_imm + '\n')
                fout.write('addi x' + rs2 + ', x' + rs2 + ', ' + rand_imm + '\n')
            fout.write(instr + ' x' + rd + ', x' + rs1 + ', x' + rs2 + '\n')
        elif instr in three_reg_b:
            rs2 = str(random.randrange(32))
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1) + ' ' + str(rs2) + '\n')
        elif instr in two_reg_b:
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1) + '\n')
        elif instr in three_reg_i:
            rnum = str(random.randrange(10))
            fout.write(instr + ' x' + rd + ', x' + rs1 + ', 0x' + rnum + '\n')
        elif instr in three_reg_i32:
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))
            shamt32 = str(random.randrange(32))
            fout.write(' ' + shamt32 + '\n')
        elif instr in three_reg_i64:
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))
            shamt64 = str(random.randrange(64))
            fout.write(' ' + shamt64 + '\n')
        elif instr == 'gorci':
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))
            fout.write(' 7\n')
        elif instr == 'grevi':
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))
            fout.write(' 56\n')
        else:
            fout.write(instr + ' x' + rd + ', x' + rs1 + '\n')

    fout.write('    mv a0, x0\n')
    fout.write('    .int 0x0000006b\n')
    fout.close()

def do_generate(case_size, case_queue, output):
    while not case_queue.empty():
        case_index = case_queue.get()
        prefix = f"_{case_index}" if case_index > 0 else ""
        target_name = f"{output}{prefix}"
        generate_asm(target_name, case_size)

def main(case_size, num_cases, num_jobs, output):
    case_queue = Queue()
    for i in range(num_cases):
        case_queue.put(i)
    process_list = []
    for _ in range(num_jobs):
        p = Process(target=do_generate, args=(case_size, case_queue, output))
        p.start()
        process_list.append(p)
    for p in process_list:
        p.join()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="random crypto tests generator")
    parser.add_argument('case_size', default=1000, type=int, help="size of each cases")
    parser.add_argument('num_cases', default=1, type=int, help='number of cases')
    parser.add_argument('--jobs', '-j', default=1, type=int, help="number of jobs")
    parser.add_argument('--output', '-o', default="crypto", type=str, help="output file")

    args = parser.parse_args()

    main(args.case_size, args.num_cases, args.jobs, args.output)
