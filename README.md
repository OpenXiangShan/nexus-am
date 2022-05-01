# The Abstract Machine (AM)

## How to build benchmark flash image
- clone `nexus-am` from github:
``` shell
git clone git@github.com:OpenXiangShan/nexus-am.git
```
- enter benchmark directory (using `apps/coremark` as an example):
```
cd /apps/coremark
make ARCH=riscv64-xs-flash
cd ./build
ls
```
- you will find a ".bin" file, this is a benchmark image in flash

## How to use the prepared flash image to do simulation
- assuming you have a `XiangShan` repo, the commit ID should be newer than 188f739de96af363761c0f2b80b95b70ad01e0fc
- make `emu` build
- use `-F` to load the image in flash:
```
./emu -F $AM_HOME//apps/coremark/build/coremark-riscv64-xs-flash.bin  -i ../ready-to-run/coremark-2-iteration.bin 2>debug.log
```
> NOTE: use `-i` to specify the initial ram image, or a ramdom file if you do not care.

## Explanation of multi-processor bring-up drivers

The driver consists of the following functions (implemented in am/src/xs/isa/riscv/mpe.c) : 

* `_mpe_init(void (*entry)())`: register an `entry` procedure that each processor will run into after initialization

* `_ncpu()`: return the total number of processors

* `_cpu()`: return the hartID of current processor (start from 0)

* `_atomic_xchg(intptr_t *addr, intptr_t newval)`: atomic exchange function，replace the value in *addr with newval and return the original value

* `_atomic_add(intptr_t *addr, intptr_t adder) `: atomic adding function，increment value in *addr with adder and return the original value

* `_barrier()`: barrier function，wait until all processors arrive here

 (For more atomic operations, You can implement it yourself using a similar format as _atomic_add) 



A simple demo is provided in tests/amtest/src/tests/mp.c  Here is the instruction to build and run

```shell
cd nexus-am
make ARCH=riscv64-xs-dual mainargs='m2'
# Here m means multi-processor demo, 2 means dual core
$(PATH_OF_DUALCORE_XS)/build/emu -i build/amtest-riscv64-xs-dual.bin --diff=$(PATH_OF_DUALCORE_XS)/ready-to-run/riscv64-nemu-interpreter-dual-so
```

The output should be like this: 

```shell
My CPU ID is 0, nrCPU is 2
My CPU ID is 1, nrCPU is 2
sum = 193 atomic_sum = 200
Finalize CPU ID: 1
sum = 193 atomic_sum = 200
Finalize CPU ID: 0
```

Note that both `sum` and `atomic_sum` are incremented 100 times per CPU parallelly. However, `atomic_sum` utilizes atomic primitive. Thus, we have `sum` <= 200 && `atomic_sum` == 200.
