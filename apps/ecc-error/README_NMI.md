# ECC Error NMI Test Suite

## 问题背景

BEU (Bus Error Unit) 产生的 ECC 错误中断是 **NMI (Non-Maskable Interrupt)**，使用 M-mode 的特殊 CSR：
- **`mnscratch` (0x740)** - NMI scratch 寄存器
- **`mnepc` (0x741)** - NMI 异常 PC
- **`mncause` (0x742)** - NMI 原因
- **`mnstatus` (0x744)** - NMI 状态

**重要**：
- NMI **共用 `mtvec`** 作为中断向量（没有单独的 `mnvec`）
- 返回时必须使用 **`mnret`** 指令，而不是 `mret` 或 `sret`

## 文件说明

### 新增文件

1. **nmi_trap.S** - NMI 汇编 trap handler
   - 保存/恢复上下文
   - 使用 NMI 专用 CSR：`mnscratch` (0x740), `mnepc` (0x741), `mncause` (0x742), `mnstatus` (0x744)
   - 使用 `mnret` 返回

2. **nmi.c** - NMI C 处理函数
   - NMI 初始化 (`_nmi_init`) - **设置 `mtvec` 而不是 `mnvec`**
   - NMI handler 注册 (`nmi_handler_reg`)
   - 主 NMI 处理逻辑

3. **nmi.h** - NMI 头文件
   - NMI 函数声明
   - NMI CSR 定义

4. **ecc_error_nmi.c** - NMI 测试用例
   - ECC 错误注入
   - NMI 处理和验证

## 编译方法

### 1. 修改 AM Makefile

在 `nexus-am/am/arch/riscv64-xs.mk` 中添加 NMI 支持：

```makefile
# 添加 NMI 源文件
SRCS-$(ISA) += \
  $(ISA)/nmi.c \
  $(ISA)/nmi_trap.S
```

### 2. 编译测试用例

```bash
cd /nfs/home/pengpeizhi/xs-env/nexus-am/apps/ecc-error
make ARCH=riscv64-xs
```

### 3. 运行测试

```bash
# 在 XiangShan 仿真环境中运行
./build/emu -i $AM_HOME/apps/ecc-error/build/ecc_error-riscv64-xs.bin --no-diff
```

## 代码要点

### NMI Handler 关键逻辑

```c
_Context *nmi_trap_handler(_Event ev, _Context *ctx) {
    // ctx->sepc 实际存储的是 mnepc
    // ctx->scause 实际存储的是 mncause
    // 设置mnepc 为下条地址

    printf("NMI: mnepc=0x%llx, mncause=0x%llx\n",
           ctx->sepc, ctx->scause);

    // 处理 NMI...
    // 清除beu产生的当前累计事件 accrued， 可能有多个事件，只清楚触发当前中断的事件
    // 把 beu 的cause 写0, 用来更新cause, cause表示事件id
    // 拉低local_interrupt 再拉高，制造一个上升沿来生成下一个事件的中断

    return ctx;  // 返回时会执行 mnret
}
```

### 汇编 Trap Handler

```asm
__am_asm_nmi_trap:
  # 保存上下文（使用 mnscratch 而不是 mscratch）
  csrrw sp, 0x740, sp  # mnscratch
  # ...

  # 读取 NMI CSR（使用数字地址）
  csrr t0, 0x742  # mncause
  csrr t1, 0x744  # mnstatus
  csrr t2, 0x741  # mnepc

  # 调用 C handler
  jal __am_nmi_handle

  # 恢复上下文（使用数字地址）
  csrw 0x744, t1  # mnstatus
  csrw 0x741, t2  # mnepc
  # ...

  # 使用 mnret 返回！
  # 由于工具链可能不支持 mnret，直接编码机器码
  .word 0x70200073  # mnret
```

### `mnret` 指令编码

由于 GNU 工具链可能不支持 `mnret` 扩展，我们使用 `.word` 伪指令直接编码：

```asm
# mnret 的机器码
.word 0x70200073

# 等价于（如果工具链支持）：
# mnret
```

**`mnret` 指令格式**：
- Opcode: `0x73` (SYSTEM)
- funct3: `0x0`
- rs1: `0x0`
- funct7: `0x702` (MNRET 的 funct12 = 0x702)
- 完整编码: `0x70200073`

**验证编码正确性**：
```bash
# 如果工具链支持 mnret，可以这样验证：
echo "mnret" | riscv64-unknown-elf-as -march=rv64imafdc_smrnmi -o /tmp/t.o
riscv64-unknown-elf-objdump -d /tmp/t.o
# 应该输出: 0:   70200073   mnret
```

**其他相关指令编码**：
- `mret`:  `0x30200073` (Machine mode return)
- `sret`:  `0x10200073` (Supervisor mode return)
- `mnret`: `0x70200073` (NMI return)

## 预期输出

```
=====================================
DCache ECC Error NMI Test Suite
=====================================

Initializing NMI handler...
NMI initialized, mnvec set to 0x...
Enabling BEU interrupts...
Enabling M-mode interrupts...
Setup complete. Starting ECC error injection tests...

Test 1: Tag ECC Error Injection
--------------------------------
Starting Tag ECC Error Injection...

=== NMI Handler ===
NMI Count: 1
mnepc   : 0x80000xxx
mncause : 0x1000
===================

Tag ECC error injection completed. val=...
NMI received!

=====================================
Test Summary:
  Total NMIs received: 1
  Last mnepc: 0x80000xxx
  Last mncause: 0x1000
=====================================
TEST PASSED
```

## 调试建议

### 波形检查信号

1. **NMI 触发**：
   - `io.csr.intrBitSet` - 应该拉高
   - BEU 中断信号

2. **PC 跳转**：
   - `mnepc` (0x741) - 保存的异常 PC
   - `mtvec` - **NMI 和 M-mode 共用的中断向量**
   - PC 应该跳到 `mtvec` 指向的地址

3. **返回**：
   - 执行 `mnret` 时，PC 应该恢复为 `mnepc` 的值
   - **不应该返回 mepc 地址**

### 常见问题

**问题 1**: PC 返回到 0 地址死循环
- **原因**: 使用了 `mret` 而不是 `mnret`
- **解决**: 确保汇编中使用 `mnret`

**问题 2**: NMI handler 没有被调用
- **原因**: `mtvec` 未正确设置
- **解决**: 检查 `_nmi_init` 是否执行，确认 `mtvec` 指向 `__am_asm_nmi_trap`

**问题 3**: 上下文恢复错误
- **原因**: 使用了错误的 CSR (mepc vs mnepc)
- **解决**: 确保使用 NMI 专用 CSR

**问题 4**: 编译错误：不认识 `mnret`
- **原因**: 工具链未启用 Smrnmi 扩展
- **解决**: 使用 `.word 0x70200073` 直接编码 `mnret` 指令

## 与普通中断的对比

| 特性 | 普通中断 (S-mode) | NMI (M-mode) |
|------|------------------|--------------|
| 向量寄存器 | `stvec` | **`mtvec` (共用)** |
| Scratch 寄存器 | `sscratch` | `mnscratch` (0x740) |
| PC 保存 | `sepc` | `mnepc` (0x741) |
| 原因寄存器 | `scause` | `mncause` (0x742) |
| 状态寄存器 | `sstatus` | `mnstatus` (0x744) |
| 返回指令 | `sret` | **`mnret`** |
| 可屏蔽 | 是 | **否** |

## 参考资料

- XiangShan NMI 文档
- RISC-V 特权级规范
- BEU (Bus Error Unit) 规范
