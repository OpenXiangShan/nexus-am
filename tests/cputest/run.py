import os
from os.path import join as pjoin
import sh
from multiprocessing import Pool, cpu_count
cputest_dir = './tests'
tests = []
num_threads = 33
for d in os.listdir(cputest_dir):
    tests.append(d[:-2])
print(tests)

p = Pool(num_threads)
def run(t):
    print(t)
    command = 'make ARCH=riscv64-noop ALL='+t+' V=OFF 2>&1 run | tee > build/'+t+'.log'
    os.system(command)
    os.system('cat build/'+t+'.log grep \"HIT\"')

p.map(run, tests)
#run(tests[0])
