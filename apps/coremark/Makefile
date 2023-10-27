NAME = coremark
SRCS = $(shell find -L ./src/ -name "*.c")
ifdef ITERATIONS
CFLAGS += -DITERATIONS=$(ITERATIONS)
endif

include $(AM_HOME)/Makefile.app
