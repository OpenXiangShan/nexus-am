# 抽象计算机

AM：C程序在“bare-metal”上的运行环境。

回顾一下C程序执行过程

```c
a.c: 

void say(const char *s);
int main() {
  say("Hello World");
}

b.c:

#include <stdlib.h>
void say(const char *s) {
  system(...);
}
```

在OS上：

```
a.c  -> 编译(gcc -c) -> a.o \ 
                            \
b.c  ->                b.o  -> 链接 -> a.out -> 加载 (OS)
```

baremetal运行的程序和OS上的程序不一样。没有libc，也没有操作系统(system会创建一个进程)，这些都是无法实现的。

baremetal上运行什么呢？操作系统——它也是个C程序。其他程序也可以。但写操作系统从来都不容易。不容易的感觉是因为OS一方面是个复杂的程序，另一方面还要和硬件打交道。

但如果我们一上来的目标就不是实现一个OS，只是让baremetal能运行一个程序呢？这就容易多了。

AbstractMachine就是从一个最小的C Runtime出发，最终支持在任何体系结构上编写纯C操作系统。

## 内存

所有东西都位于同一个地址空间里。内存的访问(读写)通过：

* 访问变量，比如(x + 1)中的x，会从内存中读取；
* 指针dereference，比如*ptr，会首先从内存中读取ptr的数值，然后再去内存地址找。

---

AM中以下内存的访问($M$)是合法的：

* 代码、静态数据：代码编译即声明的内存。
* 堆区：由heap.start和heap.end标记的一段内存，可以任意使用，初始值为零。
* 栈区：一段C函数调用使用的内存，使用栈区上空间的唯一方式是函数调用（局部变量）。栈溢出或在函数调用结束后访问栈区内存为UB。

除此之外，其他内存访问均为UB，不计入AM状态。

### 程序状态

初始时，PC为main的入口，heap为空，stack有>1KB的余量(AM保证)。

C程序的状态可以看作是(PC, R, M)。每一个C语句都有它的sequential semantics，也就是$f_{PC}(R, M) = (PC',R, M')$。$R$是C语言不可见的一些程序状态，可能是中间结果，也可能是配置。C语言程序执行完整的一条语句不会改变$R$ (但中间R可能会变化, $M$也可能是中间状态)。执行一条语句的过程会：

- read M1, M2, ...
- write M1, M2, ...
- update PC

例子：

`*ptr++` -> t := read(PTR); write(f(t) = t + 1); PC = next;

优化不改变sequential semantics，但可能会减少read/write。

---

## TRM

TRM：按照C的sequential semantics执行程序。如果执行一些非法操作导致处理器异常(例如除0)，行为是UB。

* `halt()` 终止程序的执行
* `putc(ch)`将字符ch打印到系统外(heap, stack)

---

## IOE

通过API访问外部设备。

* 如果$f_{PC}(M)​$是一个IOE API，调用后PC为下一个语句，$M​$中只有传递给的buffer会变化(从外界输入数值)，其他不变，但I/O会和物理世界发生交互。

---

## MPE

引入并行、共享内存的多个C Runtime。处理器共享代码、静态数据、堆区，但拥有独立的$PC, R$和堆栈(>1KB)。

atomc_xchg:

```c
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t oldval = *addr;
  *addr = newval;
  return oldval;
}
```

但是是“原子”的，不会被打断。non-race的semantics是UB，x@1 -> atomic@1 atomic@2 -> y@2不算race。

* 内存访问的race是UB。
* 同一个设备访问的race是UB。
* putc可以race。
* 在一个处理器上halt将终止所有处理器。

---

## CTE

引入处理器上的异常执行流

trap以后进入一个特殊的状态，依然是C程序的sequential semantics，但这时候：

1. 不允许再次trap
2. 可以返回到一个过去保存的Context，过去的Context在(PC, M)的意义上仅包含PC，但实际实现的时候可能有其他内容
3. Context中GRP1-4可以读，GRPx可以写，读写其他是UB

每个处理器会独立地trap。处理器间的trap不共享内存，因此没有race。

注意单处理器也会引起race，因为context switch。

---

## VME

允许创建一个虚拟地址空间，将一段内存(非静态、stack、heap)变为虚拟映射

* 可以把prot->pagesz大小的页面进行映射，切可以管理映射的权限(PROT_READ/WRITE/EXECUTE/NONE)
* 创建prot和ucontext，ucontext运行在低权限，看不到AM所有东西，只能看到映射后的内存

对同一个AddressSpace的操作会race (UB)。不能有两个处理器同时调度同一个AddressSpace对应的ucontext。

pgalloc和pgfree可能会race，需要保证正确性。