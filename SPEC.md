# The Abstract Machine Specification

## AbstractMachine概述

AbstractMachine (AM)是一组C API，基于这组API能实现各类系统软件(其中最重要的就是操作系统)。AM实现了机制与策略分离——AM描述了底层硬件实现系统软件所设的机制抽象，去除了指令集体系结构(ISA)设计时为硬件考虑的部分，只从软件使用的角度进行抽象。

首先，AM将直接运行在计算机系统上的程序看作是一个**静态链接、不使用任何库函数的C程序**，它从`int main()`函数开始执行，永不返回。程序的代码(code)和数据(data, bss)将被加载到内存中，并且拥有一个**至少1KB的初始堆栈**(堆栈溢出的行为未定义，非常危险，请谨慎使用)。硬件平台相关代码会将程序加载并跳转到`main`执行。

AM程序仅能访问AM API操纵硬件完成相应的功能。最基础的AM API是用于调试的接口和停机API。在此基础上，有若干组扩展API：

* IOE (I/O Extension)：I/O设备同步访问API
* ASYE (Asynchronous Extension)：中断和寄存器现场管理API
* PTE (Protection Extension)：管理虚拟地址空间管理API
* MPE (Multi-Processor Extension)：多处理器管理API

其中每一组扩展都有相应的初始化函数，例如`int ioe_init();`初始化I/O扩展。**扩展API必须在初始化后才能使用**，初始化函数的返回值代表该扩展是否存在——注意有些计算机系统可能不具有特定的扩展，例如自制的简易MIPS32 CPU不具有MMU，因此不支持PTE。

## AM中用到的数据结构

* `_Area`代表一段连续的内存，组成`[start, end)`的左闭右开区间。
* `_RegSet`代表体系结构相关的寄存器组。`GPR`系列寄存器定义了一些可以用来使用的寄存器。在中断时，`GPR1`—`GPR4`为可读寄存器。系统调用时`GPRx`为可写寄存器。例如在x86中，`#define GPR1 eax`。使用`regs->GPR1`来读取寄存器的数值。

## Turing Machine (内存计算机)

### 数据结构

* `extern _Area _heap;` 一段可读、可写、可执行的内存，作为可分配的堆区。

### TRM API

* `void _putc(char ch);` 调试输出一个字符。输出到何处由具体平台决定(例如qemu将输出到串口，因此使用`serial=stdio`就能在终端接收到调试信息；对Linux native输出到本地控制台)。
* `void _halt(int code);` 终止运行并报告返回代码。`code`为0表示正常终止。

## IO Extension (输入/输出扩展)

### 数据结构与静态数据

* `_Device`代表一个设备描述结构体，包含：
  * `uint32_t id`：设备编号；
  * `const char *name`：设备名称；
  * `size_t (*read)(intptr_t reg, void *buf, size_t size)`: 从设备控制寄存器`reg`读取`size`字节；
  * `size_t (*write)(intptr_t reg, void *buf, size_t size)`: 向设备控制寄存器`reg`写入`size`字节。

每个设备支持的读/写规范不同，请参考AM设备手册。不支持读/写的设备相应`read`/`write`为`NULL`。

### IOE API

* `int _ioe_init();` 初始化Extension，若IOE存在返回0。
* `_Device *_device(int n);`返回系统中编号为n的设备。系统中从1开始连续的设备可用(返回非`NULL`)。若系统中有10个设备，则`_device(1)`…`_device(10)`会返回非`NULL`指针，其他数值会返回空指针。

## Asynchronous Extension (异步执行扩展)

### 数据结构与静态数据

* `_Event`表示一个异步(异常/中断)事件，包含：
  * `int event`: _EVENT_XXX指定的事件编号
  * `intptr_t cause`: 产生异步事件的原因

### ASYE API

* `int _asye_init(_RegSet* (*handler)(Event ev, _RegSet *regs));`初始化Extension，若ASYE存在返回0。`handler`是监听中断/异常事件的回调函数。在中断/异常事件到来时调用`handler(ev, regs)`，`regs`是中断到来的。中断结束后将返回到返回值指定的寄存器现场(可以返回传入的参数或NULL)。系统事件：
  * `_EVENT_IRQ_TIMER`:时钟中断(无cause)
  * `_EVENT_IRQ_IODEV`:I/O设备中断(无cause)
  * `_EVENT_ERROR`:一般错误(无cause)
  * `_EVENT_PAGEFAULT`:页错误(cause是`_PROT_XXX`的值；ref是缺页的地址)
  * `_EVENT_TRAP`:内核态自陷(无cause)
  * `_EVENT_SYSCALL`: 系统调用(无cause)
* `_RegSet *_make(_Area kstack, void (*entry)(void *), void *arg);`：从`kstack`指定的内存区域中分配寄存器现场(`kstack需有足够的空间`)，其中恢复该寄存器现场将使CPU从`entry`开始执行代码，并且`entry`函数能获得参数`arg`。返回分配的寄存器现场指针(保证它在`kstack`范围内)。
* `void _yield();`：让出当前处理器的执行，使用自陷指令保存当前寄存器现场，并调用注册的handlerl`。
* `int _intr_read()`：返回当前处理器的中断状态(0关闭/1打开)。
* `int _intr_write(int enable)`：设置响应外部中断为`enable` (`0`关闭中断，其他数值打开中断)。

## Protection Extension (存储保护扩展)

### 数据结构与静态数据

* `_Protect`描述一个被保护的地址空间，包含：
  * `_Area area`: 可以使用的地址空间
  * `ptr`: 体系结构相关的指针，供AM内部使用

### PTE API

(调整中)

## Multi-Processor Extension (多处理器扩展)

### MPE API

* `int _mpe_init(void (*entry)());`启动多处理器，若MPE存在则所有系统中的处理器都调用`entry()`且不再返回，否则MPE不存在，返回非0值。
* `int _ncpu();`返回number of CPU。
* `int _cpu();`返回当前CPU的编号(从0开始)。
* `intptr_t _atomic_xchg(volatile void *addr, intptr_t newval);`原子交换两数。保证内存顺序一致性。

## 体系结构相关代码规范 (for AM developers)

每个体系结构在`arch/`中包含一个同名目录，例如`mips32-npc`, `mips32-qemu`, `x86-qemu`，第一个词是ISA，第二个词是运行平台。对于每一个体系结构目录（例如`arch/mips32-npc`），包含：

* `README.md`相关说明。
* `include/`存放相关的头文件。
* `include/arch.h`描述体系结构相关的信息，包括：
  * 整数类型`size_t`, `off_t`的定义。
  * `typedef struct _RegSet {} _Regset;`代表所有体系结构寄存器。
* `src/`存放相关的源代码文件。
* `img/`存放制作镜像必要的文件。执行`img/burn target files`能将`files`对应的文件列表(.a)链接，并烧录成名为`target`的镜像。