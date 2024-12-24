# Build bmk

Remember to set `AM_HOME` to the root of this repo.

Clear old build
```
rm -rf $AM_HOME/am/build build
```

Build latency bmk with specific footprint size: `make ARCH=riscv64-xs INST=N`,
where INST is multiplied by 256 KBytes.
Users can also use test-example.sh to generate multiple footprint sizes.

## Note for testing

If prefetcher is turned off,
to test DDR latency when L2=1M and L3=2M, we should set INST larger than 12 (=3MB).

If prefetcher is on, due the existence of temporal prefetcher,
we suggest following sizes for testing DDR latency (when L2=1M and L3=2M):
INST = 14 16 20 24 32 48


# Run

With NEMU to test building correctness:
```
$NEMU_HOME/build/riscv64-nemu-interpreter -b build/build-mem_test_latency-16-riscv64-xs.bin
```
NEMU will print something like
```
Welcome to riscv64-NEMU!
For help, type "help"
the test of load-add-load hit result
instrs 2015 cycles 2015
```
Note that `cycles` is meaningless in NEMU

Then test it with Xiangshan:
```
/path/to/xiangshan/build/emu -i build/build-mem_test_latency-16-riscv64-xs.bin
```

# Compute latency

Divide `cycles` printed by Xiangshan emu with 1000, we get the load-to-use latency.

Load-to-use latency: https://www.quora.com/What-does-it-mean-by-load-use-penalty-in-computer-architecture-Does-it-have-any-other-name

If prefetchers are turned off, Load-to-use latency consists of L1+L2+L3+Memory latency
