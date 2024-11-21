# The Abstract Machine (AM)

## Get Started for Memory Images (Workloads)

In this section, we demonstrate how to build memory images (`base_address = 0x80000000`) for simulation.

- clone `nexus-am` from github:
``` shell
git clone https://github.com/OpenXiangShan/nexus-am.git
cd nexus-am
export AM_HOME=`pwd`  # set AM_HOME
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


## How to build AM image for 16550

The major difference between RTL simulation with Verilator and FPGA of Xiangshan is that FPGA is using 16550 at 0x310b0000,
while RTL simulation is using UARTLITE at 0x40600000.
So we should change the serial devices when compiling for FPGA by
``` shell
# Hello as an example
cd apps/hello
rm -rf $AM_HOME/am/build build  # Macro definition is not yet added into dependency of Makefile, clean am.a manually
make ARCH=riscv64-xs CPPFLAGS=-DUART16550=1  # define UART16550
```
Then run `build/hello-riscv64-xs.bin` on FPGA

Remember to `rm -rf $AM_HOME/am/build build` if you want to switch back to uartlite.


## How to generate compile_commands.json

`compile_commands.json` can be used in many language servers to help completion.

``` shell
# Hello as an example
rm -rf $AM_HOME/am/build apps/hello/build  # A full rebuild to obtain full compile commands
bear -- make -C apps/hello ARCH=riscv64-xs
```

If you don't have bear on your machine, please search.
