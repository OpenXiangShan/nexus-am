include Makefile.check

AM_HOME ?= $(shell pwd)

.PHONY: all clean

all:
	@cd am && make ARCH=$(ARCH)
	@cd klib && make ARCH=$(ARCH)

clean:
	@cd am && make clean
	@cd klib && make clean
