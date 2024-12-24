ulimit -s 80000
export PATH=$RISCV/bin:$PATH
for INST in 1 2 3 4 5 6 7 8 9 10 11 12 14 16 20 24 32 40 48 56 64 72 80; do
  make ARCH=riscv64-xs INST=$INST;
  numactl -m 0 -C 0-7 $NOOP_HOME/emu-tl-2M-2b-8t --diff $NOOP_HOME/../NEMU/build/riscv64-nemu-interpreter-so  -i ./build/build-mem_test_latency-$INST-*.bin 2> err.txt
done
