# The Abstract Machine (AM)

## Get Started for Memory Images (Workloads)

In this section, we demonstrate how to build memory images (`base_address = 0x80000000`) for simulation.

- clone `nexus-am` from github:
``` shell
git clone https://github.com/OpenXiangShan/nexus-am.git
cd nexus-am
```

- enter tests or benchmark directory (using `apps/coremark` as an example):
```
cd apps/coremark
make ARCH=riscv64-xs
ls ./build # you will see .bin, .txt, *,elf
```

- use the ".bin" file as the memory image for RTL simulation

Here we use the DiffTest RTL-simulation framework as an example.
NEMU and other designs should work similarly if the base address is 0x8000_0000.

```
$NOOP_HOME/build/emu -i ./build/coremark-riscv64-xs.bin
```

## How to build benchmark flash image

Some designs have a read-only bootrom for bring-up.
This section builds the bootrom (flash) image whose base address is 0x1000_0000.
We will be using `riscv64-xs-flash` instead of `riscv64-xs` to fix into the new address space.
Do NOT follow the steps in this section if you don't know what the bootrom means.

- enter benchmark directory (using `apps/coremark` as an example):
```
cd /apps/coremark
make ARCH=riscv64-xs-flash
ls ./build
```
- you will find a ".bin" file, this is a benchmark image for flash

- to add the flash image to simulation in DiffTest, you can use the `-F` option:

```
$NOOP_HOME/build/emu -F ./build/coremark-riscv64-xs-flash.bin
```

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
