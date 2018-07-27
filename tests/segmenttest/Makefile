NAME = segmenttest
SRCS = main.c initrd.S
LIBS += klib compiler-rt
ARCH ?= mips32-npc

TESTCASE = load-store
INITRD_S = initrd.S

TESTBIN = $(AM_HOME)/tests/cputest/build/$(TESTCASE)-$(ARCH).bin

$(TESTBIN): $(AM_HOME)/tests/cputest/tests/$(TESTCASE).c
	make -s -C $(@D)/.. ARCH=$(ARCH)

$(INITRD_S): $(TESTBIN)
	@cp $< build/fsimg
	@touch $@
	echo + $@

include $(AM_HOME)/Makefile.app
