include Makefile.check

AM_HOME ?= $(shell pwd)

.PHONY: all clean

all:
	@cd am && make

# TODO: clean everything
clean:
	@cd am && make clean
