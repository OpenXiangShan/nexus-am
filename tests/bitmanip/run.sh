#!/bin/bash

# This script builds the bitmanip tests.  You should input the num of tests

if [ $# -ne 2 ]; then
    echo "Usage: $0 <num of cases in bitmanip> <num of tests>"
    echo "Example: $0 10000 10"
    exit 1
fi

for i in `seq 1 $2`
do
    cd src
    rm *.S
    python3 randtest.py $1 1
    cd ..
    make ARCH=riscv64-xs
    mv build/bitmanip-riscv64-xs.bin build/bitmanip-riscv64-xs-$i.bin
    mv build/bitmanip-riscv64-xs.elf build/bitmanip-riscv64-xs-$i.elf
done
