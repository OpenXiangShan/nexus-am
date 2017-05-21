include Makefile.check

AM_HOME ?= $(shell pwd)

.PHONY: all clean

all:
	@cd am && build ARCH=$(ARCH)
	@cd klib && build ARCH=$(ARCH)

clean:
	@cd am && make clean
	@cd klib && make clean
