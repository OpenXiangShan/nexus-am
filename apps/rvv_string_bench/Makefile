SRCS = \
	main.c \
	rvv_string.S

MARCH ?= rv64gcv_zba
N ?= 0
CASE ?= strcpy
TIMES ?= 1

VALID_CASES := strcpy strlen strcmp strchr
ifeq ($(filter $(CASE),$(VALID_CASES)),)
$(error CASE must be one of: $(VALID_CASES))
endif

CFLAGS += -DSTR_BYTES=$(N) -DTEST_$(CASE) -DTIMES=$(TIMES)
CFLAGS += -fno-tree-vectorize -fno-tree-slp-vectorize

NAME = rvv_string_bench-$(CASE)-$(N)B-$(TIMES)TIMES

include $(AM_HOME)/Makefile.app
