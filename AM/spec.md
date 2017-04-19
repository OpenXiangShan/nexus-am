# AbstractMachine Specification

## 数据结构

* `_Area`代表一段连续的内存。

* ```
  typedef struct _Area {
    void *start, *end;
  } _Area; 
  ```

* `_Screen`描述系统初始化后的屏幕（后续可通过PCI总线设置显示控制器，则此设置不再有效）。

* ```
  typedef struct _Screen {
    int width, height;
  } _Screen;
  ```

* 屏幕的像素颜色由32位整数`typedef u32 _Pixel;`确定，从高位到低位是00rrggbb，红绿蓝各8位。

* 按键代码由如下enum指定：

* ```
  enum {
    _KEY_NONE = 0,
    _KEY_UP, _KEY_DOWN, _KEY_LEFT, _KEY_RIGHT, ...
  };
  ```

* `_Protect`描述一个被保护的地址空间：

* ```
  typedef struct _Protect {
    _Area area; 
    void *ptr;
  } _Protect;
  ```

  ​

## Turing Machine

* `void _start();` loader加载完毕后，系统的入口地址。
* `void _trm_init();` 初始化Turing Machine。必须在使用任何TRM函数/数据结构前调用（只能调用一次）。
* `void _putc(char ch);` 调试输出一个字符，输出到最容易观测的地方。对qemu输出到串口，对Linux native输出到本地控制台。
* `void _halt(int code);` 终止运行并报告返回代码。`code`为0表示正常终止。
* `extern _Area _heap;` 一段可以完全自由使用的内存。

## IO Extension

* `void _ioe_init();` 初始化Extension。
* `ulong _uptime();` 返回系统启动后的毫秒数。
* `ulong _cycles();` 返回系统启动后的周期数。
* `int _peek_key();` 返回按键。如果没有按键返回`_KEY_NONE`。
* `void _draw_p(int x, int y, _Pixel p);` 在(`x`, `y`)坐标绘制像素`p`（非立即生效）。
* `void _draw_f(_Pixel *p);` 绘制W*H个像素的数组，填充整个屏幕（非立即生效）。
* `void _draw_sync();` 保证之前绘制的内容显示在屏幕上。
* `extern _Screen _screen;` 屏幕的描述信息。

## Asynchronous Extension

## Protection Extension

## Multi-Processor Extension

```
void _asye_init();
void _listen(_RegSet* (*l)(int ex, _RegSet *regs));
_RegSet *_make(_Area kstack, void *entry);
void _trap();
void _idle();
void _ienable();
void _idisable();
int _istatus();

// =======================================================================
// Protection Extension
// =======================================================================

void _pte_init(void*(*palloc)(), void (*pfree)(void*));
void _protect(_Protect *p);
void _release(_Protect *p);
void _map(_Protect *p, void *va, void *pa);
void _ummap(_Protect *p, void *va);
void _switch(_Protect *p);
_RegSet *_umake(_Area ustack, _Area kstack, void *entry, int argc, char **argv);

// =======================================================================
// Multi-processor Extension
// =======================================================================

void _mpe_init(void (*entry)());
int _cpu();
ulong _atomic_xchg(volatile ulong *addr, ulong newval);
void _barrier();
extern int _NR_CPU;

#ifdef __cplusplus
}
#endif

#endif


```