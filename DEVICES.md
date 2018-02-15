# AM Devices

[TOC]

设备相关的宏定义在`dev.h`。设备id为32位十六进制数。

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
## 0000 ac03 - AM Timer
## 0000 ac04 - AM Video
## 0000 0080 - PCI Configuration space
## 0000 0dd0 - ATA0
## 0000 0dd1 - ATA1
