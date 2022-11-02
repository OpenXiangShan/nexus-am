for ((i=0;i<3;i++))
do
  for((j=0;j<3;j++))
  do
    for((k=0;k<3;k++))
    do
      make ARCH=riscv64-xs CHOOSEO=$i CHOOSET=$j CHOOSER=$k
    done
  done
done