NAME = coremark
SRCS = $(shell find -L ./src/ -name "*.c")
ifdef ITERATIONS
CFLAGS += -DITERATIONS=$(ITERATIONS)
endif
ifdef TOTAL_DATA_SIZE
CFLAGS += -DTOTAL_DATA_SIZE=$(TOTAL_DATA_SIZE)
endif

include $(AM_HOME)/Makefile.app
