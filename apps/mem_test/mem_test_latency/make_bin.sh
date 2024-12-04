for ((i=10;i<20;i++))
do
    make ARCH=riscv64-xs INST=$i
done