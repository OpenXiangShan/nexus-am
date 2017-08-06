# RISCV32-EMULATOR

RISCV32-EMULATOR的AM实现。

## 介绍

此arch的目的就是让用chisel编写的RISCV CPU更便于仿真与调试。
这个arch的目的不是为了让riscv CPU上板跑。
传统的CPU仿真是用vivado进行的，vivado提供的功能选项很有限，
我们一般跑仿真也只能跑跑简单的测试程序，测出CPU中的明显bug。
大的测试程序需要用到外设，需要上板子跑，在这种情况下，调试是非常困难的。
[riscv-sodor](https://github.com/ucb-bar/riscv-sodor)提供的fesvr(front end server)很好的解决了这个问题。
fesvr是一个库，提供了对常用外设的模拟，包括键盘、屏幕等，并解决了CPU与外设通信的问题。
CPU访问外设主要是通过访问两个内存地址tohost与fromhost，体现在代码中就是两个uint64_t类型的变量。
CPU向外设发出命令，就是向tohost中写入值；CPU轮询fromhost值，即可以收到外设的响应。
这样子，CPU访问外设并不需要通过特殊的端口，我们不需要对CPU的设计进行改动。
另外，fesvr还提供了对系统调用的模拟，这样即使CPU未实现系统调用相关的指令，
运行于其上的代码也可以通过向tohost中写入相关值，来获得系统调用的功能。

## 仿真流程

对CPU进行仿真的整体流程是这样子的：
使用chisel编写的CPU生成出verilog，再生成出C++。我们编写的CPU的顶层模块最终变成了一个C++的类，emulator.cpp实例化这个类，并调用fesvr库，生成最终的可执行文件emulator，运行它即可进行仿真。

## 访问外设

一个输出到外设的command或者来自外设的response是一个64bit的无符号整型数。
从高byte到低byte，各个域的含义是：
|    7   |  6  | 5     0 |
| device_id | cmd | payload |
以向console输出字符'a'为例，因为console对应的是bcd设备，它的device id是1，输出的cmd是1，要输出的一字节放到payload的最低位。因此最后要写入到tohost的整型数是：0x0101000000000061。

下面是各设备的具体的接口定义：
TODO

## 内存空间layout

RISCV-sodor里面的CPU定义的内存默认是2MB，而我们这里的ld脚本默认的入口地址就是0x80000000。之所以还能运行，是因为RISCV-sodor的里定义的内存地址线宽度是与内存容量有关的，所以CPU在访存时，高位实际是被直接丢弃的。所以0x80000000实际上就是物理地址的0。类似的ld脚本定义了

0x80000000: start.S启动脚本
0x80001000: tohost
text、data。
128KB TLS(我也不知道这是啥，照着riscv-sodor里面的汇编改的)
128KB stack
剩下的都是heap area。
