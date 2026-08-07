NAME = thread_demo
SRCS = thread_demo.c
LIBS = thread
THREAD_DEMO_NCPU ?= 4
# UART output is costly in the multi-hart Verilator model. Enable it explicitly
# when running the demonstration interactively.
THREAD_DEMO_VERBOSE ?= 1
CFLAGS += -DTHREAD_DEMO_NCPU=$(THREAD_DEMO_NCPU) \
          -DTHREAD_DEMO_VERBOSE=$(THREAD_DEMO_VERBOSE)

include $(AM_HOME)/Makefile.app
