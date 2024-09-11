NAME = coremark
SRCS = $(shell find -L ./src/ -name "*.c")
ifdef ITERATIONS
CFLAGS += -DITERATIONS=$(ITERATIONS)
NAME = coremark-$(ITERATIONS)-iteration
endif

include $(AM_HOME)/Makefile.app
