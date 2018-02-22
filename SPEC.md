# The Abstract Machine Specification

注意事项：

* 所有扩展的函数/数据结构需要在扩展初始化后调用。

## AM中用到的数据结构

* `_Area`代表一段连续的内存，组成`[start, end)`的左闭右开区间。
* `_RegSet`代表体系结构相关的寄存器组。`GPR`系列寄存器定义了一些可以用来使用的寄存器(用于ABI)。`GPR1`—`GPR4`为可读寄存器，`GPRx`为可写寄存器。例如在x86中，`#define GPR1 eax`。使用`regs->GPR1`来读取寄存器的数值。

## 体系结构相关代码规范

每个体系结构在`arch/`中包含一个同名目录，例如`mips32-npc`, `mips32-qemu`, `x86-qemu`，第一个词是ISA，第二个词是运行平台。对于每一个体系结构目录（例如`arch/mips32-npc`），包含：

* `README.md`相关说明。
* `include/`存放相关的头文件。
* `include/arch.h`描述体系结构相关的信息，包括：
  * 整数类型`size_t`, `off_t`的定义。
  * `typedef struct _RegSet {} _Regset;`代表所有体系结构寄存器。
* `src/`存放相关的源代码文件。
* `img/`存放制作镜像必要的文件。执行`img/burn target files`能将`files`对应的文件列表(.a)链接，并烧录成名为`target`的镜像。

## Turing Machine (内存计算机)

### 数据结构

* `extern _Area _heap;` 一段可读、可写、可执行的内存，作为可分配的堆区。

### TRM API

* `void _putc(char ch);` 调试输出一个字符。输出到何处由具体平台决定(例如qemu将输出到串口，因此使用`serial=stdio`就能在终端接收到调试信息；对Linux native输出到本地控制台)。
* `void _halt(int code);` 终止运行并报告返回代码。`code`为0表示正常终止。

## IO Extension (输入/输出扩展)

### 数据结构与静态数据

* `_Device`代表一个设备描述结构体，包含：
  * `uint32_t id`：设备编号
  * `const char *name`：设备名称
  * `intptr_t (*read)(intptr_t reg, size_t nmemb)`: 调用函数将读取设备控制寄存器，编号为reg，字节数为nmemb。
  * `void (*write)(intptr_t reg, size_t nmemb, intptr_t data)`: 调用函数将写入设备控制寄存器，编号为reg，字节数为nmemb。

### IOE API

* `void _ioe_init();` 初始化Extension。
* `_Device *device(int n);`返回系统中编号为n的设备。系统中从1开始连续的设备可用(返回非NULL)。

## Asynchronous Extension (异步执行扩展)

### 数据结构与静态数据

* `_Event`表示一个异步(异常/中断)事件，包含：
  * `int event`: _EVENT_XXX指定的事件编号
  * `inptr_t cause`: 产生异步事件的原因

### ASYE API

* `void _asye_init(_RegSet* (*l)(Event ev, _RegSet *regs));`初始化Extension。初始化后并不响应异步事件。`l`是监听中断/异常事件的回调函数。在中断/异常事件到来时调用l(ev, regs)。中断结束后将返回到返回值指定的寄存器现场(可以返回传入的参数或NULL)。系统事件：
  * `_EVENT_IRQ_TIMER`:时钟中断(无cause)
  * `_EVENT_IRQ_IODEV`:I/O设备中断(无cause)
  * `_EVENT_ERROR`:一般错误(无cause)
  * `_EVENT_PAGENP/_EVENT_PAGEPROT`:缺页/页保护错(cuase = `_PG_R`/`_PG_W`表示发生页错误的是读/写操作；ref存放产生缺页的地址)
  * `_EVENT_TRAP`:内核态自陷(无cause)
  * `_EVENT_SYSCALL`: 系统调用(无cause)
* `_RegSet *_make(_Area kstack, void *entry, void *arg);`创建一个内核上下文,参数arg。
* `void _trap();`在内核态自陷。线程需要睡眠/让出CPU时使用。
* `int _istatus(int enable);`设置中断状态(enable非0时打开)。返回设置前的中断状态(0/1)。

## Protection Extension (存储保护扩展)

### 数据结构与静态数据

* `_Protect`描述一个被保护的地址空间，包含：
  * `_Area area`: 可以使用的地址空间
  * `ptr`: 体系结构相关的指针，供AM内部使用

### PTE API

* `void _pte_init(void*(*palloc)(), void (*pfree)(void*));`初始化Extension。传入两个函数，分别代表分配/释放一个物理页(分配需保证多线程/多处理器安全)。
* `void _protect(_Protect *p);` 创建一个保护的地址空间。
* `void _release(_Protect *p);` 释放一个保护的地址空间。
* `void _map(_Protect *p, void *va, void *pa, int mode);`将地址空间的虚拟地址va映射到物理地址pa。单位为一页。
* `void *_query(_Protect *p, void *va, int *mode);` 获取虚拟地址va所对应的物理页。相应的权限(`_PG_R`/`_PG_W`/`_PG_X`保存在`mode`中，如果`mode==NULL`则不保存)。
* `void _unmap(_Protect *p, void *va);`释放虚拟地址空间va对应的映射页。
* `void _switch(_Protect *p);`切换到一个保护的地址空间。注意在内核态下，内核代码将始终可用。
* `_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, int argc, char **argv, char **envp);`创建一个在低权限运行的地址空间。`p`必须被`_protect`，`argv`, `envp`必须指向用户栈中的内容。(地址空间p，用户栈地址ustack，内核栈地址kstack，入口地址entry, 其余为用户进程参数).

## Multi-Processor Extension (多处理器扩展)

### MPE API

* `void _mpe_init(void (*entry)());`启动多处理器。
* `int _ncpu();`返回number of CPU。
* `int _cpu();`返回当前CPU的编号(从0开始)。
* `intptr_t _atomic_xchg(volatile void *addr, intptr_t newval);`原子交换两数。保证内存顺序一致性。
