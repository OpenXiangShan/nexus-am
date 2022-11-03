for ((i=0;i<3;i++))
do
  for((j=0;j<2;j++))
  do
    for((k=0;k<2;k++))
    do
      for((ii=0;ii<2;ii++))
      do
        for((jj=0;jj<2;jj++))
        do
            make ARCH=riscv64-xs CHOOSEO=$i CHOOSET=$j CHOOSER=$k CHOOSEF=$ii CHOOSEI=$jj
        done
      done
    done
  done
done