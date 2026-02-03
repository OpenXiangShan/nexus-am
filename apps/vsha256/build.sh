set -x

export AM_HOME=$PWD/../..

iterations=(
  1 
  # 10 
)

sizes=(
  # 0
  55
  128 256 512 1024 2048 4096 8192 16384 32768 
  65536 131072 262144 524288 1048576 2097152
  4194304
)

for iter in ${iterations[@]}; do
  for size in ${sizes[@]}; do
    make ARCH=riscv64-xs CROSS_COMPILE=$HOME/opt/RISCV/bin/riscv64-unknown-elf- N=${size} ITERATIONS=${iter}
    rm -rf build/riscv64-xs
  done
done

