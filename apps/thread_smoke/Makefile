NAME = thread_smoke
SRCS = thread_smoke.c
LIBS = thread
# Run the multi-hart coverage check by default; single-hart remains overrideable.
THREAD_SMOKE_NCPU ?= 4
CFLAGS += -DTHREAD_SMOKE_NCPU=$(THREAD_SMOKE_NCPU)

include $(AM_HOME)/Makefile.app
