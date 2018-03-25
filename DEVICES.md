# AM Devices

设备相关的宏定义在`amdev.h`。设备id为32位十六进制数。

**对于读/写寄存器是结构体的，size必须与结构体一致，否则行为未定义**。PCI Conf Space、ATA控制器等支持不同数量字节的读写。

| 设备ID      | 设备功能                                     | 设备描述    |
| --------- | ---------------------------------------- | ------- |
| 0000 ac01 | 若干性能相关的计数器(ro)。           | AM性能计数器 |
| 0000 ac02 | 键盘控制器(#1, ro)。 | AM输入设备 |
| 0000 ac03 | 系统启动时间(#1, ro)和实时日期(#2, ro)。 | AM定时器   |
| 0000 ac04 | 屏幕信息(#1, ro)和显存控制(#2, wo)。 | AM显示控制器 |
| 0000 0080 | PCI Configuration Space (bus, slot, func, offset, rw)。 | PCI控制器  |
| 0000 0dd0 | ATA主控制器(#1-#7, rw)。       | ATA控制器0 |
| 0000 0dd1 | ATA从控制器(#1-#7, rw)。         | ATA控制器1 |

## 0000 ac01 - AM PerfCnt
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
