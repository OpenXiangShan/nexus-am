ulimit -s 80000
export RISCV=/nfs/share/riscv-toolchain-gcc15-240613-noFFnoSeg2
export PATH=$RISCV/bin:$PATH
for INST in 1 2 4 6 8 10 12 16 24 32 40 48 64 80 96 112 128; do
  make ARCH=riscv64-xs INST=$INST;
  numactl -m 0 -C 0-7 $NOOP_HOME/emu-tl-2M-2b-8t --diff $NOOP_HOME/../NEMU/build/riscv64-nemu-interpreter-so  -i ./build/build-mem_test*-$INST-*.bin 2> err.txt | tee -a log.txt;
done
