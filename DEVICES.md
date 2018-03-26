# AM Devices (`amdev.h`)

## AM设备API概述

`_Device`结构体描述了一个设备(通过`_device(n)`获取)，结构体中包含：

* `uint32_t id`：设备编号，32位整数；
* `const char *name`：设备的名称(调试用)；
* `size_t (*read)(intptr_t reg, void *buf, size_t size)`: 从设备控制寄存器`reg`读取`size`字节；
* `size_t (*write)(intptr_t reg, void *buf, size_t size)`: 向设备控制寄存器`reg`写入`size`字节。

理解设备最重要的概念是设备的**控制寄存器**：我们将设备看成是一系列的可以读/写的**寄存器**，从寄存器中读出数据或写入数据即实现对设备的读/写访问。设备寄存器有以下两种可能：

1. 一个有含义的结构体。读取/写入的数据必须与结构体的大小严格一致(`read`/`write`的`size`参数)。结构体用于AM虚拟设备。
2. 一个端口/内存映射的地址。此时可以向其中写入1, 2, 4, 8等字节的数值。例如一个8位的数据端口可以写入`write(reg, &ch, 1);`，其中`ch`是`uint8_t`类型。

## AM设备列表

| 设备ID      | 设备名称                                 | 设备寄存器与功能描述 (r = read, w = write, o = only) |
| --------- | ---------------------------------------- | ------- |
| 0000 ac01 | AM性能计数器    | 若干性能相关的计数器(ro)。           |
| 0000 ac02 | AM输入设备 | 键盘控制器(#1, ro)。 |
| 0000 ac03 | AM定时器 | 系统启动时间(#1, ro)和实时日期(#2, ro)。 |
| 0000 ac04 | AM显示控制器 | 屏幕信息(#1, ro)和显存控制(#2, wo)。 |
| 0000 0080 | PCI控制器 | PCI Configuration Space (bus, slot, func, offset, rw)。 |
| 0000 0dd0 | ATA控制器0 | ATA主控制器(#1-#7, rw)。       |
| 0000 0dd1 | ATA控制器1  | ATA从控制器(#1-#7, rw)。         |

## 0000 ac01 - AM PerfCnt

目前还没有这个虚拟设备。

## 0000 ac02 - AM Input

1.  (`_DEVREG_INPUT_KBD`): AM键盘控制器。从中读出`_KbdReg`结构体，`keydown`为是否按下；`keycode`为扫描码(`_KEY_XXX`)。没有按键时`keycode = _KEY_NONE`。

## 0000 ac03 - AM Timer

1. (`_DEVREG_TIMER_UPTIME`): AM系统启动时间。从中读出`_UptimeReg`结构体，`(hi << 32LL) | lo`是系统启动的毫秒数。
2. (`_DEVREG_TIMER_DATE`): AM实时时钟(Real Time Clock)。从中读出`_DateReg`，包含年月日时分秒。

## 0000 ac04 - AM Video

1. (`_DEVREG_VIDEO_INFO`): AM显示控制器信息。从中读出`_VideoInfoReg`结构体。其中`width`为屏幕宽度、`height`为屏幕高度。假设AM运行过程中屏幕大小不发生变化。
2. (`_DEVREG_VIDEO_FBCTL`): AM帧缓冲控制器，向其中写入`_FBCtlReg`结构体，向屏幕(`x`,`y`)坐标处绘制`w`*`h`的矩形图案，像素按行优先存储在`pixels`。每个32位整数以`00RRGGBB`描述颜色。

## 0000 0080 - PCI Configuration space
## 0000 0dd0 - ATA0
## 0000 0dd1 - ATA1
