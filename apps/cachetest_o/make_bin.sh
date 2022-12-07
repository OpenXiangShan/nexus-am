for ((j=3;j<6;j++))
do
    for ((i=0;i<51;i++))
    do
        make ARCH=riscv64-xs CHOOSE=$j INST=$i
    done
done
