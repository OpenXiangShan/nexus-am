# AM 线程库

[English](README.md)

`libs/thread` 是面向 AM 和香山多核仿真的轻量级 POSIX 风格逻辑线程库。一个
hart 同一时刻只运行一个逻辑线程；多个 hart 从同一个就绪队列取任务。实现位于
`src/thread.c`，公共接口位于 `include/thread.h`。

## 功能与模型

- 线程生命周期：`thread_create`、`thread_join`、`thread_detach`、
  `thread_exit`、`thread_self` 和 `thread_yield`。
- 同步原语：普通、递归和 error-check mutex，条件变量、屏障和 `thread_once`。
- 逻辑线程局部存储：`thread_key_*`，在线程退出时最多执行四轮析构函数。
- 全局 FIFO 调度：所有 hart 共用一个就绪队列，入队在尾、出队在头。
- `riscv64-xs-dual` 支持定时器抢占；其他架构仍是协作式调度，CPU 密集型代码
  必须主动调用 `thread_yield()`。

线程 ID 从 1 开始；ID 0 是主程序上下文。线程没有 `home_cpu` 或公开亲和性
属性，某次 yield 后可以由另一个 hart 继续执行。`thread_attr_t` 中的 priority
仅为 API 兼容保留，不影响 FIFO 顺序。

## 为什么 `main()` 可直接调用线程 API

线程库采用惰性初始化。第一次 `thread_create()` 或 `thread_yield()` 会在 CPU 0
执行 `ensure_scheduler()`：

1. 安装 CTE yield handler；在 `riscv64-xs-dual` 上同时安装 timer handler。
2. 调用 `_mpe_start(thread_secondary_entry)`，发布从核入口并等待每个选中从核
   完成自身调度器初始化。
3. 将从核标记为空闲。后续创建线程时，任务进入共享 FIFO，调度器从空闲从核中
   按轮询顺序挑选一个并发送通知。

所以应用没有额外的 `thread_init()`。但是必须在第一次线程 API 调用前配置实际
硬件核数，例如四核仿真：

```c
_mpe_setncpu('4');
```

参数必须与仿真硬件一致，并在 `1..MAX_CPU` 内。AM 当前的 `MAX_CPU` 为 8；
链接脚本为每个 hart 预留 128 KiB 栈/TLS 区域。这是实现和链接布局上限，不代表
已经完成八核验收。

## 多 hart 启动与唤醒

`_mpe_wakeup(cpu)` 需要处理两种不同状态：

1. 第一次启动目标 hart 时，向
   `HART_CTRL_RESET_REG_BASE + cpu * 8` 写 0，使其离开复位。
2. 每次需要通知目标 hart 时，向
   `CLINT MSIP[cpu] = 0x38000000 + cpu * 4` 写 1。

复位寄存器只能负责首次启动，不能唤醒已经运行或已经停在等待点的 hart；后者需要
MSIP。香山当前 emu 中 S 态 `wfi` 对 MSIP 的恢复并不稳定，线程库的空闲路径因此
轮询共享就绪队列并做短暂 backoff。MSIP 仍保留为正确的硬件通知机制，复位释放也
只执行一次。

创建线程时，调度器在持锁状态下把线程放到 FIFO 尾部，并从空闲从核位图中选择一个
hart。它会设置该 hart 的 `wake_pending`，形成一个短暂的 handoff：其他 hart 在
handoff 被选中 hart 消费前不取队头，避免创建者立即抢回刚分派的任务。handoff 完成
后，任一进入调度器的 hart 都可以取得队首，因此线程的具体落核不保证，也不应该从
日志顺序推断固定绑定关系。

## 上下文切换和时钟抢占

调用 `thread_yield()`、在同步原语上等待，或收到时钟事件时，调度器会保存当前线程
上下文。仍然可运行的线程重新入队，取得 FIFO 队首并恢复它的上下文。恢复时会用当前
hart 的 `tp` 覆盖目标上下文的 `tp`，因此逻辑线程跨 hart 恢复不会误用另一个 hart
的 TLS 基址。

在 `riscv64-xs-dual` 上，每个 hart 有独立的 `mtimecmp` 和 `mscratch` timer 状态。
线程库在 CTE 切换到 S 态前把时钟周期设为 200 微秒，完整路径如下：

```text
CLINT mtimecmp
  -> M 态 timer vector
  -> 带来源标记的 SSIP
  -> CTE 识别为 timer event
  -> thread_timer_handler
  -> FIFO 调度器
```

M 态向下一次 compare 推进并触发 SSIP；CTE 将带标记的 SSIP 分发给注册的 timer
handler。某 hart 已经持有调度器锁时收到 tick 会直接返回原上下文，以避免调度器重入；
下一次 tick 再尝试调度。时钟处理要求的 S 态中断开关由线程库设置。该库拥有 AM timer
callback，且会关闭外部中断；应用若需要额外 timer 或外部中断处理，必须与 CTE 分发器
集成，不能直接覆盖线程库回调。

S 态逻辑线程打印时应使用 `s_atomic_printf`，其锁会保存并恢复 `sstatus.SIE`。
原有 `atomic_printf` 继续使用 `mstatus` 锁定路径，供启动和 M 态代码保持原有行为。

## 最小使用示例

```c
#include <am.h>
#include <thread.h>

static void *worker(void *arg) {
  return arg;
}

int main(void) {
  _mpe_setncpu('4');

  thread_t thread;
  void *result;
  thread_create(&thread, NULL, worker, (void *)42);
  thread_join(thread, &result);
  return result == (void *)42 ? 0 : 1;
}
```

应用 Makefile 中加入：

```make
LIBS += thread
```

默认线程栈为 16 KiB，最小为 4 KiB。TCB 和栈从 AM 堆分配，当前 bare-metal 实现
不回收它们，包括 detached 线程；可创建的线程数因此受堆空间限制。

## 构建与验证

在仓库根目录执行：

```sh
export AM_HOME="$PWD"
make -C libs/thread ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1
make -C apps/thread_smoke ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1 THREAD_SMOKE_NCPU=4
make -C apps/thread_demo ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1 THREAD_DEMO_NCPU=4
make -C tests/threadtest ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1
```

- `apps/thread_smoke` 是回归测试：覆盖创建/退出/join、mutex、屏障、条件变量、
  超时、`thread_once`、TSD 析构、所有配置 hart 的 worker 覆盖，以及无主动 yield
  的抢占检查。抢占阶段会创建比 hart 数多一个的忙等 worker；全部完成证明时钟中断
  发生了上下文切换。
- `apps/thread_demo` 是交互式示例。较长的多核仿真可用
  `THREAD_DEMO_VERBOSE=0` 减少 UART 输出。
- `tests/threadtest` 是无限输出的观察型压力程序，只用于查看定时器切换，不是会
  自行结束的回归测试。

现有 reference model 只支持单核和双核。四核香山 emu 必须禁用差分测试：

```sh
/path/to/emu -i apps/thread_smoke/build/thread_smoke-riscv64-xs-dual.bin \
  --no-diff
```

## 当前限制

- 没有优先级调度、公开 CPU 亲和性 API、取消机制或 TCB/线程栈动态回收。
- `thread_key_*` 是逻辑线程 TLS；C 语言 `__thread` 仍是物理 hart 本地存储，不能
  作为逻辑线程局部变量使用。
- 条件变量和屏障通过 yield 等待，不维护私有阻塞队列。
- 非 `riscv64-xs-dual` 目标的 CPU 密集型线程必须显式 yield。
- 四核验证需要 `--no-diff`；当前差分参考模型不支持四核。
