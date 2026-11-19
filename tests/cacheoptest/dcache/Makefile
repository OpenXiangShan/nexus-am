NAME := dcacheop-test
SRCS := $(shell find -L ./src/ -name "*.[cS]")

ifneq ($(MAKECMDGOALS),clean)
ifeq ($(filter riscv64-xs%,$(ARCH)),)
$(error dcacheop-test requires XiangShan cache-op CSRs; ARCH=$(ARCH) is not supported)
endif
endif

include $(AM_HOME)/Makefile.app
