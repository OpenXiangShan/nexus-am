NAME := llcop-test
SRCS := $(shell find -L ./src/ -name "*.[cS]")

ifneq ($(MAKECMDGOALS),clean)
ifeq ($(filter riscv64-xs%,$(ARCH)),)
$(error llcop-test requires XiangShan HuanCun control registers; ARCH=$(ARCH) is not supported)
endif
endif

include $(AM_HOME)/Makefile.app
