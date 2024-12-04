# build bmk

Remember to set `AM_HOME` to the root of this repo.

Clear old build
```
rm -rf $AM_HOME/am/build build
```

Build bandwidth bmk:
```
make ARCH=riscv64-xs
```

# Build DUT with well-tuned prefetcher

To fully utilize the memory bandwidth, this benchmark is assuming well-tuned spatial prefetchers, such as
stream prefetcher, stride prefetcher, or BOP.

If the prefetcher is not well-tuned, this benchmark is just stressing other part of the CPU,
like instruction window size, MHSR counts.
Then this benchmark will be nonsense.

# Run

With NEMU to test building correctness:
```
$NEMU_HOME/build/riscv64-nemu-interpreter -b build/mem_test_bw-riscv64-xs.bin
```

Then test it with Xiangshan:
```
/path/to/xiangshan/build/emu -i build/mem_test_bw-riscv64-xs.bin
```

Xiangshan with single channel DDR4 3200 configuration is expected to print something like
```
start ddr test
mem band width 7.145648 B/cycle (2000 samples) inst 6016, checksum=0
Core 0: HIT GOOD TRAP at pc = 0x80000288
```

# Compute bandwidth

Memory bandwidth = 7.145648 B/cycle * 3GHz = 21.435 GB/s

3GHz is the simulated frequency of CPU
