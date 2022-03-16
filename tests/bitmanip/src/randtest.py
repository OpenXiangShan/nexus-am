#! /usr/bin/env python3

import argparse
import random
import subprocess
from multiprocessing import Process, Queue

# Generate riscv-bitmanip assembly code
# Need to modify the file($AM_HOME/am/arch/isa/riscv64.mk) as follow:
# -CROSS_COMPILE := riscv64-linux-gnu-
# -COMMON_FLAGS  := -fno-pic -march=rv64g -mcmodel=medany
# +CROSS_COMPILE := /bigdata/zfw/b-tools/bin/riscv64-unknown-linux-gnu-
# +COMMON_FLAGS  := -fno-pic -march=rv64gc_zba_zbb_zbc_zbs -mcmodel=medany
#
# Run "python3 randtest.py" to generate random test
# Run "make ARCH=riscv64-noop" to compile

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

def generate_asm(out, test_size):
    asm_name = f"{out}.S"

    fout = open(asm_name, 'w')

    fout.write('#include "extB.S.include"\n')
    fout.write('.section .text.init\n')
    fout.write('.global _trm_init\n')
    fout.write('_trm_init:\n')
    fout.write('    j START_RAMDOM_TEST\n')
    fout.write('START_RAMDOM_TEST:\n')

    def randinit_reg(r):
        if r != 0:
            rand_data = hex(random.randrange(2 ** 64))
            fout.write('li x' + str(r) + ',' + str(rand_data) + '\n')
        return

    print(f"Generate {test_size} cases to {asm_name}")

    for i in range(0, test_size):
        instr = random.choice(instructions)
        rd = random.randrange(31) + 1  # skip x0
        rs1 = random.randrange(32)
        randinit_reg(rs1)

        if instr in three_reg:
            rs2 = str(random.randrange(32))
            randinit_reg(rs2)
            fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))
            fout.write(' ' + str(rs2) + '\n')
            continue

        fout.write(instr + ' ' + str(rd) + ' ' + str(rs1))

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

    fout.write('    mv a0, x0\n')
    fout.write('    .int 0x0000006b\n')
    fout.close()

def compile_asm(target_name):
    run_cmd = ["make", "ARCH=riscv64-xs", "-C", "..", f"NAME={target_name}"]
    proc = subprocess.Popen(run_cmd, stdout=subprocess.DEVNULL, stderr=None)
    while proc.poll() is None:
        pass

def do_generate(case_size, case_queue, output, do_compile, compile_queue):
    while not case_queue.empty():
        case_index = case_queue.get()
        prefix = f"_{case_index}" if case_index > 0 else ""
        target_name = f"{output}{prefix}"
        generate_asm(target_name, case_size)
        if do_compile:
            compile_queue.put(target_name)

def main(case_size, num_cases, num_jobs, output, do_compile):
    case_queue, compile_queue = Queue(), Queue()
    for i in range(num_cases):
        case_queue.put(i)
    process_list = []
    for _ in range(num_jobs):
        p = Process(target=do_generate, args=(case_size, case_queue, output, do_compile, compile_queue))
        p.start()
        process_list.append(p)
    for p in process_list:
        p.join()
    while not compile_queue.empty():
        target = compile_queue.get()
        print(f"Compiling {target} ...")
        compile_asm(target)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="random bitmanip tests generator")
    parser.add_argument('case_size', default=1000, type=int, help="size of each cases")
    parser.add_argument('num_cases', default=1, type=int, help='number of cases')
    parser.add_argument('--jobs', '-j', default=1, type=int, help="number of jobs")
    parser.add_argument('--output', '-o', default="bitmanip", type=str, help="output file")
    parser.add_argument('--compile', '-C', action='store_true', default=False, help='compile the test cases')

    args = parser.parse_args()

    main(args.case_size, args.num_cases, args.jobs, args.output, args.compile)
