# HPM driver用法

## 环境

&emsp; &emsp; [nexus-am仓库地址](https://github.com/OpenXiangShan/nexus-am/tree/southlake)，采用`southlake`分支。用法见[开发环境文档](https://xiangshan-doc.readthedocs.io/zh_CN/latest/tools/xsenv/)

```shell
nexus-am/apps/hpmdriver
├── Makefile
├── events.h		 ：性能事件编码表
├── hpmdriver.h	：头文件，包含性能时间寄存器结构和各类功能函数
├── hpmdriver.c	：主函数
```

&emsp; &emsp; 采用`make ARCH=riscv64-xs-southlake`编译，可以采用如下指令编译＋运行

```shell
make ARCH=riscv64-xs-southlake && $NOOP_HOME/build/emu --no-diff -i ./build/*.bin 2>&1 | tee log | less
```

## 使用方法

&emsp; &emsp; 向性能事件寄存器`mhpmevent`中写入`希望统计的性能事件`，以及`多个性能事件之间的组合方式`（可以采用头文件中的宏定义）。然后读取对应的`mhpmcounter`寄存器，即可获得计数值。

&emsp; &emsp; mode域指定，统计该特权级下的性能事件。

&emsp; &emsp; 性能计数事件的组合方式：

- ​	 Event0 \<Optype0\> Event1 = T1
- ​	 Event2 \<Optype1\> Event3 = T2
- ​	 T1 \<Optype2\> T2 = Result

&emsp; &emsp; **特别注意，性能事件必须用其对应的性能事件寄存器统计**，具体见下表。因为各个Block的`Hardware Performance Monitor`只与对应范围的`csr mhpmevent`寄存器相连，所以只能利用对应范围的`mhpmevent`寄存器去统计。而且各个Block性能事件的编码均从0开始，如果没有利用正确的`mhpmevent`寄存器，会导致统计结果出错。

| 事件  |  对应的寄存器   |
| ------------- | ---- |
| Frontend | `mhpmevent3~10` |
| CtrlBlock | `mhpmevent11~18` |
| MemBlock       | `mhpmevent19~26` |
| 缓存      | `mhpmevent27~31` |

&emsp; &emsp; 提供了 `set_event_quad/set_evet_double/set_event_single`来设置4个性能事件的组合/2个性能事件的组合/单个性能事件。`se_cc`设置性能事件并清除计数器。`print_coutner`打印计数器的值。

&emsp; &emsp; 具体示例，可以参考`hpmdriver.c`

