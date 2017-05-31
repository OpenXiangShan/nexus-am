include Makefile.check

AM_HOME ?= $(shell pwd)

.PHONY: all clean

all:
	@cd am && make ARCH=$(ARCH)
	@cd libs/klib && make ARCH=$(ARCH)
	@cd libs/imgui && make ARCH=$(ARCH)

clean:
	@cd am && make clean
	@cd libs/klib && make clean
	@cd libs/imgui && make clean
