CROSS_COMPILE := riscv64-unknown-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=rv64gc_zba_zbb_zbc_zbs -mcmodel=medany
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
