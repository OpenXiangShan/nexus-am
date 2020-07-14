for t in $(ls tests);
do
  echo ${t%.c}
  make ARCH=riscv64-noop ALL=${t%.c} run 2>&1 | tee > build/${t%.c}.log
  cat build/${t%.c}.log | grep "HIT GOOD TRAP"
done
