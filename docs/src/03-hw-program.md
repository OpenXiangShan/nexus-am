# 在计算机硬件上编程

以前学的写C程序，都是写完，按一个键(编译运行)，在OS上运行。

但C也可以在硬件上运行。

听起来很可怕，这里有一个思维翻转的过程：我们觉得在机器上编程很可怕，因为机器有很多细节(specs, 约定)我们都不知道。

但其实我们(system)要做的事情，就是做抽象，把复杂的东西简化了，做成C runtime。

## Minimal C Runtime

就是刚才说的，能执行semantics。

多一个putchar()。

这个就能干很多事了。

## Playing with AM (minimal C runtime)

例子：自己写了个CPU，非常简陋。

但可以做很多事。[^hw-1]

[^hw-1]: Hello.

::::: {.sidebar}
MIPS32 implementation
:::::

::::: {.sidebar}
x86 implementation
:::::