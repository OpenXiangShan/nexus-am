# /bin/bash

set -x

# u can set something here
TESTNUM=40
# ITERS=4096
TESTNAME="h264refhotloop"
GEM5HOME="/nfs/home/lixin/myworkspace/simulator/bankconflict/GEM5"
DIFF="/nfs-nvme/home/share/zhenhao/ref-h/build/riscv64-nemu-interpreter-so"
# EXTRAARG="--l1d_assoc 8 --ideal-kmhv3"
EXTRAARG="--bp-type=DecoupledBPUWithBTB --ideal-kmhv3"

COMASTART=17
COMAEND=45

# compile test cases

# iteration 0
sed -i "s/#define[[:space:]]\+PERIET[[:space:]]\+[0-9]\+/#define PERIET 1/" main.c
sed -i "$COMASTART,${COMAEND}s/^/#/" asm.S
make ARCH=riscv64-xs
mv build/h264refhotloop-riscv64-xs.bin build/h264refhotloop-riscv64-xs-iter-0.bin

sed -i "$COMASTART,${COMAEND}s/^#//" asm.S

for (( i=1; i<$TESTNUM; i++ ))
do
    echo "Generation Iteration $i"
    sed -i "s/^#define PERIET .*$/#define PERIET $i/" main.c
    make ARCH=riscv64-xs
    mv build/$TESTNAME-riscv64-xs.bin build/$TESTNAME-riscv64-xs-iter-$i.bin
done

set +x
# set environment
export GCBV_REF_SO=$DIFF
export GEM5_HOME=$GEM5HOME
# run it
for (( i=0; i<$TESTNUM; i++ ))
do
    echo "Run Iteration $i"
    $GEM5HOME/build/RISCV/gem5.opt --outdir=build/out$i $GEM5HOME/configs/example/xiangshan.py --generic-rv-cpt build/$TESTNAME-riscv64-xs-iter-$i.bin --raw-cpt $EXTRAARG
done

# collect result and analyze

echo "+-------------------------------------------------------------------------------- Summary --------------------------------------------------------------------------------+"
printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" 'Iteration' 'instrCnt' 'cycleCnt' 'ipc for all iterations' 'delta instrCnt' 'delta cycleCnt' 'delta ipc'
for (( i=0; i<$TESTNUM; i++ ))
do
    if [ $i -gt 0 ]; then
        preInstrCnt=$(cat build/out$((i - 1))/stats.txt | grep "commit.instsCommitted" | awk '{print $2}')
        preCycleCnt=$(cat build/out$((i - 1))/stats.txt | grep "cpu.numCycles" | awk '{print $2}')
        instrCnt=$(cat build/out$i/stats.txt | grep "commit.instsCommitted" | awk '{print $2}')
        cycleCnt=$(cat build/out$i/stats.txt | grep "cpu.numCycles" | awk '{print $2}')
        ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
        ipcIter=$(echo "($instrCnt - $preInstrCnt) / ($cycleCnt - $preCycleCnt)" | bc -l)
        printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" $i $instrCnt $cycleCnt $ipcAll $((instrCnt - preInstrCnt)) $((cycleCnt - preCycleCnt)) $ipcIter
    else
        instrCnt=$(cat build/out$i/stats.txt | grep "commit.instsCommitted" | awk '{print $2}')
        cycleCnt=$(cat build/out$i/stats.txt | grep "cpu.numCycles" | awk '{print $2}')
        ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
        printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" $i $instrCnt $cycleCnt $ipcAll 0 0 0
    fi
done

echo "+-------------------------------------------------------------------------------- Summary --------------------------------------------------------------------------------+"