# The Abstract Machine (AM)

抽象计算机：必要的抽象，帮你编写一个完整的计算机系统！

目录组织：

* `am/`：AM头文件、每个体系结构分别实现的AM代码。
* `klib/`：建立在AM上、体系结构无关的基础运行库，提供诸如`memcpy`等基础功能。
* `apps/`：基于AM实现的各类应用。

参考：[AM Specification](am/spec.md)。