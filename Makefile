include Makefile.check

AM_HOME ?= $(shell pwd)

.PHONY: all clean

all:
	@$(MAKE) -C am

# TODO: clean everything
clean:
	@$(MAKE) -C am clean
