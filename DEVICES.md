# AM Devices

[TOC]

设备相关的宏定义在`dev.h`。设备id为32位十六进制数。

| 设备ID      | 设备功能                                     | 设备描述    |
| --------- | ---------------------------------------- | ------- |
| 0000 ac01 | 若干32位寄存器，编号从0开始                          | AM性能计数器 |
| 0000 ac02 | Read(0) get scancode, Write(0) putchar                    | AM Console  |
| 0000 ac03 | 从0号寄存器读出系统启动到当前的毫秒数(32bit)               | AM定时器   |
| 0000 ac04 | 向1 2 3 4 5写入参数，向0写入非0数值绘图。向0写入0重绘屏幕      | AM显示控制器 |
| 0000 0080 | 寄存器为configuration space地址(bus, slot, func, offset组成) | PCI控制器  |
| 0000 0dd0 | ?                                        | ATA控制器0 |
| 0000 0dd1 | ?                                        | ATA控制器1 |

## AM PerfCnt (0000 ac01)

## AM Console (0000 ac02)

```
#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define _KEY_NAME(k) _KEY_##k,

enum {
  _KEY_NONE = 0,
  _KEYS(_KEY_NAME)
};
```

## AM Timer (0000 ac03)
## AM Video (0000 ac04)
## PCI Configuration space (0000 0080)
## ATA0 (0000 0dd0)
## ATA1 (0000 0dd1)
