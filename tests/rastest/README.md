# RAS Test

Tests RISC-V Return Address Stack update behavior by comparing normal `call`/`ret` traffic with a `jalr` coroutine pop-and-push pattern.

- clone `nexus-am` from github:
``` shell
git clone https://github.com/OpenXiangShan/nexus-am.git
cd nexus-am
export AM_HOME=`pwd`  # set AM_HOME
export CROSS_COMPILE=riscv64-linux-gnu-
```

- enter tests or benchmark directory:
```
cd test/rastest
make ARCH=riscv64-xs
ls ./build # you will see .bin, .txt, *,elf
```