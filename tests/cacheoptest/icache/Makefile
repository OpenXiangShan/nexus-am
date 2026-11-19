NAME := icacheop-test
SRCS := $(shell find -L ./src/ -name "*.[cS]")

ifneq ($(MAKECMDGOALS),clean)
ifeq ($(filter riscv64-xs%,$(ARCH)),)
$(error icacheop-test requires XiangShan cache-op CSRs; ARCH=$(ARCH) is not supported)
endif
endif

include $(AM_HOME)/Makefile.app
