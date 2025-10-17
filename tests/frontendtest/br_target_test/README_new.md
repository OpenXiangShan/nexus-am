# 前端分支目标预测测试（br_target_test）说明

本说明文档汇总 `tests/frontendtest/br_target_test` 下前端“分支目标预测”相关测试，用于后续模拟器与 RTL 的对齐与问题定位。涵盖每个测试的含义、测试目的、关注的预测器（BTB/间接目标预测器/返回地址栈 RAS 等）、建议观测点与对齐方法。

## 1. 环境与构建
- 位置：`tests/frontendtest/br_target_test`
- 依赖：要求设置 `AM_HOME`，并根据平台设置 `ARCH`（如 `native`、`riscv64-qemu` 等）。
- 构建命令（示例）：
  - `cd tests/frontendtest/br_target_test`
  - 默认 `make ARCH=riscv64-xs` 编译全部测试
  - `make ARCH=riscv64-xs ALL=<test-name>`（例如：`make ARCH=riscv64-xs ALL=call_branch`）
- 产物：`build/<ARCH>/<test-name>-<ARCH>` 可用于在各自平台/runner 上运行与采样统计。
- 备注：本目录下 Makefile 将测试按 `tests/*.c` 自动枚举，默认 `-O0`，并使用 `__attribute__((noinline))` 与 `volatile` 抑制优化以保留典型控制流形态。

## 2. 测试清单与说明

以下每个小节均给出：测试目标、分支类型、重点关注的预测器、生成的行为模式、对齐/观测建议。

### 2.1 call_branch（直接调用目标）
- 源码：`tests/frontendtest/br_target_test/tests/call_branch.c`
- 分支类型：函数直接调用（call，目标为链接期已知的直接目标）。
- 关注预测器：
  - 早取向前重定向能力（BTB 的直接目标供给）
  - 非返回路径上通常不依赖 RAS（返回时才用到 RAS）
- 行为模式：
  - 16 周期循环模式，按 4×4 分段依次调用 `func1`/`func2`/`func3`/`func4`。
  - 实际执行的是 4 个不同的调用点（不同 PC），分别指向 4 个固定目标入口。
- 测试目的：
  - 验证 BTB/前端是否能对“直接调用”在取指阶段给出正确目标，减少 decode 解析目标带来的气泡。
  - 检查microBTB 是否能正确预测目标，并给出正确的目标地址(4个target 足够存储了)。
- 对齐与观测建议：
  - 统计每个调用点的 BTB hit/miss、预测目标与实际目标是否一致；关注是否存在别名冲突导致偶发 miss。
  - 记录每次 call 后的返回点 PC（用于与 2.4 的 RAS 测试联动诊断）。

### 2.2 return_branch（返回指令目标）
- 源码：`tests/frontendtest/br_target_test/tests/return_branch.c`
- 分支类型：函数返回（ret）。
- 关注预测器：返回地址栈 RAS（Return Address Stack）。
- 行为模式：
  - 同一个被调函数 `func_for_ret_test()`，从 4 个不同调用点交替调用后返回。
  - 要求前端在 ret 时弹出正确返回地址，实现“最近调用优先”的 LIFO 语义。
- 测试目的：
  - 验证 RAS 的正确性与稳健性：push/pop 时机、栈深管理、异常/冲刷场景下是否保持一致性。
- 对齐与观测建议：
  - 统计 RAS 命中率、push/pop 次数是否与调用/返回数量匹配。
  - 观察分支冲刷（mispredict/异常）时 RAS 是否被正确回滚；若对齐错误，通常表现为返回落点混乱或间歇性错误。

### 2.3 indirect_branch（间接调用/跳转目标）
- 源码：`tests/frontendtest/br_target_test/tests/indirect_branch.c`
- 分支类型：通过函数指针进行间接调用（RISC-V 上通常为 `jalr`）。
- 关注预测器：间接分支目标预测（ITTAGE 预测结果）；若仅有 BTB（单目标），会出现高错率。
- 行为模式：
  - 4 个目标函数，以 `i % 4` 的 round-robin 轮转调用（多目标、多模态）。
- 测试目的：
  - 评估前端对“多目标间接分支”的学习与选择能力；无专用间接预测器时，应观察到显著的 thrash/miss。
- 对齐与观测建议：
  - 针对同一分支 PC，记录预测目标与实际目标分布，统计按历史模式选择的目标是否逐步收敛。
  - 关注是否有路径历史（GHR/PC/上下文）参与索引/选择器，及别名导致的跨点干扰。

### 2.4 jump_branch（无条件跳转目标）
- 源码：`tests/frontendtest/br_target_test/tests/jump_branch.c`
- 分支类型：函数内 `goto` 到局部标签（通常编译为直接无条件跳转）。
- 关注预测器：
  - 直接跳转目标的早期供给（BTB）与 decode 求值路径的一致性与性能差异。
- 行为模式：
  - 通过 `switch (i % 4)` 选择 4 处 `goto targetX`，各标签紧随本函数内。
- 测试目的：
  - 验证对“直接无条件跳转”的目标提供是否稳定；评估 BTB 命中与潜在的别名冲突。
- 对齐与观测建议：
  - 记录取指阶段是否已重定向至目标（BTB 命中），以及 decode 后的目标是否与早取一致。
  - 若设计选择仅在 decode 解析无条件跳转目标，也应记录对应的 front-end 泡影/气泡差异用于性能对齐。

## 3. 统一观测指标（建议）
为便于模拟器与 RTL 对齐，建议在两端统一采集以下事件统计或波形标记：
- BTB：查表命中/未命中、给出的目标、更新位置与替换策略生效情况；按“分支 PC”聚合统计。
- RAS：push/pop 次数、栈深随时间变化、命中/回退事件、异常/冲刷时的回滚是否一致。
- 间接预测：同一分支 PC 的多目标分布、历史相关性（GHR/路径/返回上下文）是否生效、选择器/仲裁命中率。
- 全局：预测目标 vs 实际目标的逐次对比日志（PC、预测目标、实际目标、是否 flush）。

## 4. 结果判定与期望
- call_branch / jump_branch：在短暂预热后，应趋近 100% 目标命中（若仅 decode 求值，也应在功能上完全一致）。这两个测试程序比较一致，结果可能也一致。
- return_branch：在无嵌套/无异常干扰场景下，RAS 命中率应接近 100%；若出现错配，多与 RAS 回滚/深度管理相关。
- indirect_branch：
  - 具备专用“间接目标预测”时，应对周期性 4 目标模式逐步学习并提升命中率；
  - 若仅 BTB（单目标），则会出现明显的错配（多目标 thrash），可用于验证“无专用间接预测器”的基线表现。

## 5. 已知注意事项
- 默认 `-O0`，并通过 `noinline/volatile` 降低优化干扰，便于观察“典型控制流形态”。
- 测试本身不进行 I/O；如需计数统计，请在平台侧增加事件计数器/trace 钩子，不要修改测试以免改变控制流。
- `jump_branch` 的 `goto` 属于函数内直接跳转，通常不依赖方向预测；不同微架构可能在取指阶段使用 BTB 做“提前重定向”，或在 decode 阶段解析立即数——请据实现选择观测点。

## 6. 参考源码映射
- call：`tests/frontendtest/br_target_test/tests/call_branch.c`
- ret：`tests/frontendtest/br_target_test/tests/return_branch.c`
- indirect：`tests/frontendtest/br_target_test/tests/indirect_branch.c`
- jump：`tests/frontendtest/br_target_test/tests/jump_branch.c`

## 7. 待补充测试
- 测试BTB 容量，使用直接跳转or goto 语句，当分支数目超过micro BTB 时候，override bubble 次数应该大幅提升，当超过ahead BTB 时候，提升，当超过main BTB 时候还会提升。