for ((i=0;i<4;i++))
do
  for((j=0;j<5;j++))
  do
    make ARCH=riscv64-xs CHOOSEO=$i CHOOSET=$j
  done
done