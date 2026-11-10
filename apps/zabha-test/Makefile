NAME = zabha-test
SRCS = zabha-test.c

# The C source uses Zabha/Zacas mnemonics directly, so build it with a toolchain
# new enough to recognize them, such as the GCC 15 nightly under /nfs/share.
MARCH ?= rv64gc_zabha_zacas

# Zabha adds AMOCAS.B/H only when Zacas is also implemented.
WITH_ZACAS ?= 1
ifeq ($(WITH_ZACAS),1)
NAME = zabha-test-zacas
DST_DIR = $(APP_DIR)/build/$(ARCH)-zacas/
CFLAGS += -DZABHA_WITH_ZACAS
endif

# The dual-core target also runs contention and acquire/release tests.
ifneq ($(findstring -dual,$(ARCH)),)
CFLAGS += -DZABHA_SMP
endif

include $(AM_HOME)/Makefile.app
