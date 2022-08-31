ifeq ($(LINUX_GNU_TOOLCHAIN),1)
CROSS_COMPILE := riscv64-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=rv64gc -mcmodel=medany
else
CROSS_COMPILE := riscv64-unknown-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=rv64gc_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt -mcmodel=medany
endif
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
