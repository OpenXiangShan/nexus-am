# 条件分支预测测试（cond_br_test）说明

本说明文档汇总 `tests/frontendtest/cond_br_test` 下前端“条件分支方向预测”相关测试，用于后续模拟器与 RTL 的对齐与问题定位。为每个测试给出：测试含义、测试目的、重点关注的预测器（方向预测/BHT-PHT/选择器/TAGE/GShare 等）与对齐建议。

## 1. 环境与构建
- 位置：`tests/frontendtest/cond_br_test`
- 依赖：要求设置 `AM_HOME`，并根据平台设置 `ARCH`（如 `native`、`riscv64-qemu` 等）。
- 构建（示例）：
  - `cd tests/frontendtest/cond_br_test`
  - `make ARCH=riscv64-xs ALL=<test-name>`（例如：`make ARCH=riscv64-xs ALL=alternating_test`）
- 产物：`build/<ARCH>/<test-name>-<ARCH>`，可在目标平台/runner 上运行并配合统计/trace。
- 默认编译：`-O0`，源码大量使用 `__attribute__((noinline))` 与简单整数循环，避免编译器将分支优化消除，便于观察典型方向预测行为。

## 2. 测试清单与说明
以下每个小节均给出：分支模式、目的、关注组件、对齐/观测建议。

### 2.1 always_taken_test（永远为真）
- 源码：`tests/frontendtest/cond_br_test/tests/always_taken_test.c`
- 模式：条件恒成立（始终 Taken）。
- 目的：验证方向预测器对“单一稳定模式”的快速学习与稳态命中率；检查冷启动/切换影响。
- 关注：BHT/PHT 饱和计数器是否迅速收敛到强 Taken；TAGE/GShare 的分配与命中路径。
- 建议：统计前若干次迭代的 miss 情况与稳态准确率；观察替换/分配是否按预期触发。

### 2.2 never_taken_test（永远为假）
- 源码：`tests/frontendtest/cond_br_test/tests/never_taken_test.c`
- 模式：条件恒不成立（始终 Not-Taken）。
- 目的：与 2.1 对偶，验证强 NT 学习与稳态；检查偏置方向的对称性。
- 关注：计数器向强 NT 收敛速度、冷启动偏置；选择器是否偏向某一路。
- 建议：同 2.1，记录冷启动与稳态对比。

### 2.3 alternating_test（周期=2 的交替）
- 源码：`tests/frontendtest/cond_br_test/tests/alternating_test.c`
- 模式：T N T N ... 周期 2。
- 目的：检验短周期可预测性与 GHR 参与的模式学习；2 位饱和计数器在该模式下的局限。
- 关注：基于历史的预测（GShare/TAGE）是否利用 PC⊕GHR 捕捉周期 2；简单局部计数器易被来回翻转。
- 建议：统计稳定期准确率、观察是否因别名导致错配。

### 2.4 two_bit_pattern_test（TT NN 交替）
- 源码：`tests/frontendtest/cond_br_test/tests/two_bit_pattern_test.c`
- 模式：TT NN TT NN ...（周期 4）。
- 目的：考察更长周期模式下的历史利用能力；验证表项位宽/历史深度对可预测性的影响。
- 关注：含历史的预测器（TAGE/局部历史表）能否学到“2T 后 2N”的重复模式。
- 建议：观察学习收敛时间、稳态准确率与别名影响。

### 2.5 three_bit_pattern_test（TTT NNN 交替）
- 源码：`tests/frontendtest/cond_br_test/tests/three_bit_pattern_test.c`
- 模式：TTT NNN TTT ...（周期 6）。
- 目的：与 2.4 类似，但周期更长；验证较长历史/多表融合策略的收益。
- 关注：TAGE 中长表/选择器介入时机；若历史深度不足则准确率受限。
- 建议：同 2.4，额外关注不同 history 长度表的命中占比（若可观测）。

### 2.6 prime_based_pattern_test（素数线性同余伪随机）
- 源码：`tests/frontendtest/cond_br_test/tests/prime_based_pattern_test.c`
- 模式：`(i*7+13)%19<10`，看似随机但确定性。
- 目的：评估对“非简单周期”模式的可学习性；作为难例基线。
- 关注：复杂相关性在有限表项与选择器下的学习效果；是否退化到近似 50% 准确率。
- 建议：记录长时间窗口准确率、是否出现阶段性提升或震荡。

### 2.7 switching_pattern_test（相位切换：80%→20%）
- 源码：`tests/frontendtest/cond_br_test/tests/switching_pattern_test.c`
- 模式：前半段 80% Taken，后半段 20% Taken。
- 目的：检验预测器对分布突变的自适应能力（去学习与再学习速度）。
- 关注：计数器/历史在切换点后的回退速度；选择器反转时的过渡开销。
- 建议：以时间轴标注切换点，统计切换前后窗口内的准确率与收敛步数。

### 2.8 nested_branches_test（嵌套分支）
- 源码：`tests/frontendtest/cond_br_test/tests/nested_branches_test.c`
- 模式：两层条件嵌套（i%3 与 i%5 / i%7），存在相关性与路径依赖。
- 目的：考察路径相关性与多分支互扰；验证全局历史对相关分支的建模能力。
- 关注：GHR 更新策略、路径长度对可分辨性的影响；别名与表项竞争。
- 建议：分别对内外分支计数，观察互相影响导致的精度波动。

### 2.9 early_exits_test（循环提前退出）
- 源码：`tests/frontendtest/cond_br_test/tests/early_exits_test.c`
- 模式：外层 for 叠加“满足 (i>n/2 且 i%17==0) 则 break”。
- 目的：检验“循环退出”类分支的可预测性；路径长度与稀疏触发对学习的影响。
- 关注：对稀疏触发的条件能否逐步学习；退出触发后的 flush 行为一致性。
- 建议：记录触发点附近的准确率、是否存在过拟合或误触发。

### 2.10 rare_branches_test（低概率分支）
- 源码：`tests/frontendtest/cond_br_test/tests/rare_branches_test.c`
- 模式：约 1% 概率 Taken（i%100==97）。
- 目的：评估低频 Taken 时，预测器是否保持“短期稳态 NT”而不过度震荡。
- 关注：计数器阈值/饱和速度；低概率事件后的恢复速度。
- 建议：统计长窗口准确率、Taken 触发后连续若干次预测的轨迹。

### 2.11 gradual_transition_test（渐变从全 T 到全 N）
- 源码：`tests/frontendtest/cond_br_test/tests/gradual_transition_test.c`
- 模式：随 i 增大，Taken 概率单调下降。
- 目的：检验缓慢漂移分布下的自适应与稳定性；对比 2.7 的突变场景。
- 关注：是否存在过度反应或滞后；不同表的权重切换平滑性。
- 建议：滑动窗口准确率曲线应平滑过渡，无长时间震荡。

### 2.12 aliasing_pattern_test（别名/冲突压力）
- 源码：`tests/frontendtest/cond_br_test/tests/aliasing_pattern_test.c`
- 模式：两阶段在“可能别名的地址集合”上施加相反模式，制造表项冲突。
- 目的：评估表项索引/标记、组相联与替换策略；验证别名下的退化行为。
- 关注：BHT/PHT 命中率明显下降、不同阶段互相污染；选择器是否能部分缓解。
- 建议：统计两个阶段各自稳态准确率；若可观测，记录参与替换的表、Victim 选择。

### 2.13 all_patterns_test（综合运行）
- 源码：`tests/frontendtest/cond_br_test/tests/all_patterns_test.c`
- 模式：依次调用上述所有模式函数，形成长程序。
- 目的：综合检验“多模式切换 + 历史污染 + 选择器仲裁”的整体稳健性。
- 关注：不同模式之间的跨段干扰、学习/去学习开销累积；长程序中的全局趋势。
- 建议：分段采集各小节准确率，并关注相邻小节切换处的恢复速度。

## 3. 统一观测指标（建议）
- 方向预测：逐分支 PC 的预测结果/实际结果（T/NT）、mispredict 标记、flush 次数与触发阶段。
- 历史/表项：GHR 更新轨迹、BHT/PHT 饱和计数器状态迁移；TAGE/GShare 各表命中/分配/替换统计。
- 选择器：不同来源（全局/局部/默认）的仲裁命中率与决策变化。
- 冲突/别名：索引冲突率、各组占用与替换分布；跨模式污染对精度的影响。

## 4. 结果判定与期望（参考）
- always/never：短暂预热后趋近 100%；冷启动少量 miss。
- alternating / 两位/三位模式：具备有效历史建模时应显著优于 50%；历史不足或别名重会退化。
- prime_based：难例，准确率可能接近随机；强相关结构（更深历史）可有提升。
- switching / gradual：切换点/缓变区的收敛速度与稳定性是关键；不应长时间震荡。
- rare / early exit：低频 Taken 或提前退出后应快速回到稳态，避免长时间误预测。
- aliasing：预期出现退化，用于验证索引/标记/替换策略是否合理。

## 5. 注意事项
- 测试均为纯 C、无 I/O；请在平台侧增加事件计数/trace 钩子，不要修改测试本身以免改变控制流。
- 这些测试主要针对“方向预测”；目标预测（BTB/目标地址）在这些 if-branch 中通常为“顺序 or 固定偏移”，非此处重点。
- 编译/平台差异：保持 `-O0` 可避免把分支优化掉；若更改编译选项，请先检查生成的分支形态是否仍符合预期。
