include Makefile.check

AM_HOME ?= $(shell pwd)

all:
	@$(MAKE) -s -C am

# clean everything
ALLMAKE = $(dir $(shell find . -mindepth 2 -name "Makefile"))
$(ALLMAKE):
	-@$(MAKE) -s -C $@ clean

clean: $(ALLMAKE)

.PHONY: all clean $(ALLMAKE)
