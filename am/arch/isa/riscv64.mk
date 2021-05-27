CROSS_COMPILE := /opt/riscv64b/bin/riscv64-unknown-elf-
COMMON_FLAGS  := -fno-pic -march=rv64gb -mcmodel=medany
# CROSS_COMPILE := riscv64-linux-gnu-
# COMMON_FLAGS  := -fno-pic -march=rv64gc -mcmodel=medany
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
