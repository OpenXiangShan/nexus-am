HAS_RVV=0

ifeq ($(LINUX_GNU_TOOLCHAIN),1)
RV_MARCH = rv64gc
CROSS_COMPILE := riscv64-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=$(RV_MARCH) -mcmodel=medany
else ifeq ($(HAS_RVV),1)
RV_MARCH = rv64gcv_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt
CROSS_COMPILE := /nfs/home/share/riscv-v/bin/riscv64-unknown-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=$(RV_MARCH) -mriscv-vector-bits=256 -mcmodel=medany
else
RV_MARCH = rv64gc_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt
CROSS_COMPILE := riscv64-unknown-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=$(RV_MARCH) -mcmodel=medany
endif
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
