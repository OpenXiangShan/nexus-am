# 异常处理与Handler注册

## 异常处理流程

### RISC-V异常处理模型概述

RISC-V处理器的异常在默认情况下一律陷入到machine态，machine态软件视情况决定直接处理或转交supervisor态处理。为减少整体trap次数以提升常见中断异常的处理速度，RISC-V还提供了中断和异常代理功能。符合代理条件的异常可不经machine态直接转交supervisor态处理。

### AM在该异常处理模型下的实现细节

AM在RISC-V处理器上运行时，复位在machine态，在完成machine态的异常handler设置及异常代理、CLINT、PMP等必须在machine态完成的初始化设置后，进入supervisor态并设置supervisor态的异常handler。

machine态的异常handler实际仅处理时钟中断和非法指令两种异常。时钟中断异常在machine态利用更新mtimecmp csr的方式清除后，machine态handler会主动设置supervisor态的外部中断异常以便supervisor态软件得知中断触发并处理。考虑到supervisor态无法主动清除machine态所设置的supervisor态外部中断异常信号，为其在machine态提供了一个利用非法指令清除外部中断信号的方法。machine态handler在检测到非法指令异常时，会将supervisor态外部异常信号清空并返回触发异常指令的下一条指令执行。

supervisor态是AM用户接触最多的状态。为兼顾通用性及各功能处理由自身产生的各项异常需求，AM在supervisor态提供了分两层的运行时动态注册异常handler功能。

具体地，supervisor态异常handler在保存现场后会进入C语言`__am_irq_handle`函数，该函数根据scause寄存器最高位判断异常是否为中断并分别进入异常和中断handler表中对应cause所注册的异常handler。用户可使用`irq_handler_reg`函数注册相应handler。该级handler是可由用户主动注册的第一级handler。在默认情况下，时钟中断handler被注册为`__am_irq_STIP_handler`，外部中断handler被注册为`__am_irq_SEIP_handler`，ecall异常handler被注册为`__am_irq_SECALL_handler`，其他类型异常handler被注册为`__am_irq_default_handler`。

考虑到amtest中大量测试需要主动触发外部中断、ecall异常并自行对异常做进一步的处理，SEIP、SECALL、STIP三个handler还进一步提供了自定义handler注册机制。用户可使用`custom_handler_reg`函数按照异常编号进行自定义handler注册或直接调用`seip_handler_reg`、`secall_handler_reg`及`stip_handler_reg`函数注册相应handler。这类handler一旦注册，将会在默认handler执行结束后被调用。这一级handler是可由用户主动注册的第二级handler。

## handler注册示例与说明

`test/amtest`目录下的测试充分利用了上述机制，可用于参考。

main.c中在调用每个具体测试前，首先使用CTE宏完成中断机制初始化，而后利用REEH、RCEH、RTEH等宏分别调用外部中断、ecall异常和时钟中断的自定义handler注册，最终再调用测试本身。

此外，`src/nemu/isa/riscv/cte.c`中`_cte_init`函数对`irq_handler_reg`函数的调用可用作该函数用法参考。

### handler实现需求

第一级handler执行前，supervisor态软件并未对该异常做任何具体处理，一切与此异常处理具体相关的操作都要在该handler内实现完毕，通常包括清除中断信号（以避免重复进入该异常的处理）、设置相关event、设置特权态csr等。第二级handler执行前，supervisor态软件往往已对该异常进行了必要处理并生成了该异常具体原因（event），二级handler一般仅需要根据该event信息修改部分全局变量值、进行某些输出等操作。

第一级handler可参考`src/nemu/isa/riscv/cte.c`中`__am_irq_SEIP_handler`实现，第二级handler可参考`tests/amtest/tests/intr.c`中`simple_trap`实现。