for ((i=1;i<13;i++))
do
    make ARCH=riscv64-xs CHOOSE=$i
done
