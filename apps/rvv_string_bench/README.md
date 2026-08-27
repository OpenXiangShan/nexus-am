# rvv_string_bench

面向 RISC-V Vector（RVV）的字符串函数微基准，用于测量向量化字符串函数在不同
输入长度下的执行周期和吞吐率。每次构建生成一个固定测试函数、字符串长度和调用
次数的 `riscv64-xs` 镜像。

## 测试内容

输入字符串由 `N` 个字符 `A` 和结尾的 `\0` 组成。输入准备和向量状态初始化不计入
测量；程序在两次 `mcycle` 读数之间执行 `TIMES` 次函数调用，周期数包含外层循环
开销。

| `CASE` | 实现函数 | 测试行为 |
| --- | --- | --- |
| `strcpy` | `vector_strcpy` | 将源字符串复制到目标缓冲区 |
| `strlen` | `vector_strlen` | 查找字符串结尾并返回长度 |
| `strcmp` | `vector_strcmp` | 比较两份内容相同的字符串 |
| `strchr` | `vector_strchr` | 查找字符 `0`，即查找结尾的 `\0` |

建议使用以下长度进行测试：

```text
0, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
65536, 131072, 262144, 524288, 1048576, 2097152, 4194304
```

`CASE`、`N` 和 `TIMES` 都是编译时参数，默认值分别为 `strcpy`、`0` 和 `1`。
`CASE` 可选 `strcpy`、`strlen`、`strcmp`、`strchr`；`N` 为非负整数；`TIMES` 为
正整数。文件名格式如下：

```text
rvv_string_bench-<case>-<bytes>B-<times>TIMES-riscv64-xs.bin
```

例如，`CASE=strlen N=4096 TIMES=10` 会生成：

```text
build/rvv_string_bench-strlen-4096B-10TIMES-riscv64-xs.bin
```

## 构建

需要准备 AM 环境和支持 RVV 的 `riscv64-unknown-elf` 交叉工具链。将 `AM_HOME` 设置
为本仓库根目录：

```sh
export AM_HOME=/path/to/nexus-am
```

在应用目录中执行 `make`：

```sh
make -C "$AM_HOME/apps/rvv_string_bench" \
  AM_HOME="$AM_HOME" \
  ARCH=riscv64-xs \
  CROSS_COMPILE=/path/to/riscv-gnu-toolchain/bin/riscv64-unknown-elf- \
  MARCH=rv64gcv_zba \
  CASE=strlen N=4096 TIMES=10
```

构建产物包括 `.bin`、`.elf` 和反汇编文本，位于：

```text
apps/rvv_string_bench/build/
```

切换 `CASE`、`N` 或 `TIMES` 后重新构建时，建议先清理该应用的旧产物：

```sh
make -C "$AM_HOME/apps/rvv_string_bench" clean
```

## 文件说明

| 文件 | 作用 |
| --- | --- |
| `Makefile` | 设置架构、测试函数、字符串长度和调用次数，并生成 AM 镜像 |
| `main.c` | 初始化输入、读取 `mcycle` 并输出性能结果 |
| `rvv_string.S` | RVV 字符串函数和向量化输入填充实现 |
