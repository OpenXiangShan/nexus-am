# AM Devices

[TOC]

设备相关的宏定义在`dev.h`。设备id为32位十六进制数。

**对于读/写寄存器是结构体的，size必须与结构体一致，否则行为未定义**。

| 设备ID      | 设备功能                                     | 设备描述    |
| --------- | ---------------------------------------- | ------- |
| 0000 ac01 | 若干32位寄存器，编号从0开始                          | AM性能计数器 |
| 0000 ac02 |                                                     | AM Input  |
| 0000 ac03 | 从0号寄存器读出系统启动到当前的毫秒数(32bit)               | AM定时器   |
| 0000 ac04 | 向1 2 3 4 5写入参数，向0写入非0数值绘图。向0写入0重绘屏幕      | AM显示控制器 |
| 0000 0080 | 寄存器为configuration space地址(bus, slot, func, offset组成) | PCI控制器  |
| 0000 0dd0 | ?                                        | ATA控制器0 |
| 0000 0dd1 | ?                                        | ATA控制器1 |

## 0000 ac01 - AM PerfCnt
## 0000 ac02 - AM Input

从`_DEV_INPUT_REG_KBD` (#1)中读出32位整数，其中，`_KEY_XXX`为keyup事件；`_KEY_XXX | 0x8000`为keydown事件。

## 0000 ac03 - AM Timer

从`_DEV_TIMER_REG_UPTIME` (#1)中读出如下结构体：
```
typedef struct _Dev_Timer_Uptime {
  uint32_t hi, lo;
} _Dev_Timer_Uptime;
```

## 0000 ac04 - AM Video

从 `_DEV_VIDEO_REG_INFO` (#1)中读出如下结构体，其中`width`为屏幕宽度、`height`为屏幕高度。假设AM运行过程中屏幕大小不发生变化：
```
typedef struct _Dev_Video_Info {
  int32_t width, height;
} _Dev_Video_Info;
```

向`_DEV_VIDEO_REG_FBCTL` (#2)中写入如下结构体，向(`x`,`y`)坐标处绘制`w`*`h`的矩形图案，像素按行优先存储在`pixels`。每个32位整数以`00RRGGBB`描述颜色：
typedef struct _Dev_Video_FBCtl {
  int x, y, w, h, sync;
  uint32_t *pixels;
} _Dev_Video_FBCtl;

## 0000 0080 - PCI Configuration space
## 0000 0dd0 - ATA0
## 0000 0dd1 - ATA1
