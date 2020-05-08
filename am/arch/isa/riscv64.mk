CROSS_COMPILE := riscv-none-embed-
COMMON_FLAGS  := -fno-pic -march=rv64imfd -mabi=lp64d -mcmodel=medany
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
