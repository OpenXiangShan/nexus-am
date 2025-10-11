# /bin/bash

set -x

TESTNUM=12

COMASTART=17
COMAEND=45
# COMASTART=12
# COMAEND=18

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
    sed -i "s/#define[[:space:]]\+PERIET[[:space:]]\+[0-9]\+/#define PERIET $i/" main.c
    make ARCH=riscv64-xs
    mv build/h264refhotloop-riscv64-xs.bin build/h264refhotloop-riscv64-xs-iter-$i.bin
done

set +x
# run it
for (( i=0; i<$TESTNUM; i++ ))
do
    echo "Run Iteration $i"
    ./build/emu -i build/h264refhotloop-riscv64-xs-iter-$i.bin --no-diff -s 1322 2>h264refhotloop-iter-$i.err
done

# collect result and analyze

# for (( i=0; i<$TESTNUM; i++ ))
# do
#     echo "---------------- Iteration $i ----------------"
#     if [ $i -gt 0 ]; then
#         preInstrCnt=$(cat memcpyPerf-iter-$((i - 1)).err | grep commitInstr, | awk '{print $NF}')
#         preCycleCnt=$(cat memcpyPerf-iter-$((i - 1)).err | grep clock_cycle, | awk '{print $NF}')
#         instrCnt=$(cat memcpyPerf-iter-$i.err | grep commitInstr, | awk '{print $NF}')
#         cycleCnt=$(cat memcpyPerf-iter-$i.err | grep clock_cycle, | awk '{print $NF}')
#         echo "instrCnt: $instrCnt"
#         echo "cycleCnt: $cycleCnt"
#         ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
#         echo "ipc for all $i iterations: $ipcAll"
#         echo "delta instrCnt: $((instrCnt - preInstrCnt))"
#         echo "delta cycleCnt: $((cycleCnt - preCycleCnt))"
#         ipcIter=$(echo "($instrCnt - $preInstrCnt) / ($cycleCnt - $preCycleCnt)" | bc -l)
#         echo "ipc for The $((i))th iteration: $ipcIter"
#     else
#         instrCnt=$(cat memcpyPerf-iter-$i.err | grep commitInstr, | awk '{print $NF}')
#         cycleCnt=$(cat memcpyPerf-iter-$i.err | grep clock_cycle, | awk '{print $NF}')
#         echo "instrCnt: $instrCnt"
#         echo "cycleCnt: $cycleCnt"
#         ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
#         echo "ipc for The $((i))th iteration: $ipcAll"
#     fi
#     echo "---------------- ----------- ----------------"
# done

echo "+-------------------------------------------------------------------------------- Summary --------------------------------------------------------------------------------+"
printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" 'Iteration' 'instrCnt' 'cycleCnt' 'ipc for all iterations' 'delta instrCnt' 'delta cycleCnt' 'delta ipc'
for (( i=0; i<$TESTNUM; i++ ))
do
    if [ $i -gt 0 ]; then
        preInstrCnt=$(cat h264refhotloop-iter-$((i - 1)).err | grep commitInstr, | awk '{print $NF}')
        preCycleCnt=$(cat h264refhotloop-iter-$((i - 1)).err | grep clock_cycle, | awk '{print $NF}')
        instrCnt=$(cat h264refhotloop-iter-$i.err | grep commitInstr, | awk '{print $NF}')
        cycleCnt=$(cat h264refhotloop-iter-$i.err | grep clock_cycle, | awk '{print $NF}')
        ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
        ipcIter=$(echo "($instrCnt - $preInstrCnt) / ($cycleCnt - $preCycleCnt)" | bc -l)
        printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" $i $instrCnt $cycleCnt $ipcAll $((instrCnt - preInstrCnt)) $((cycleCnt - preCycleCnt)) $ipcIter
    else
        instrCnt=$(cat h264refhotloop-iter-$i.err | grep commitInstr, | awk '{print $NF}')
        cycleCnt=$(cat h264refhotloop-iter-$i.err | grep clock_cycle, | awk '{print $NF}')
        ipcAll=$(echo "$instrCnt / $cycleCnt" | bc -l)
        printf "%-10s | %-20s | %-20s | %-30s | %-30s | %-30s | %-30s\n" $i $instrCnt $cycleCnt $ipcAll 0 0 0
    fi
done

echo "+-------------------------------------------------------------------------------- Summary --------------------------------------------------------------------------------+"