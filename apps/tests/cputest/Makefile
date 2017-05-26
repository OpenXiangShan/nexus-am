include $(AM_HOME)/Makefile.check
.PHONY: all clean latest

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))
	@echo "" $(ALL)

Makefile.%: tests/%.c latest
	@echo "Build $*"
	@echo "NAME = $*\nSRCS = $<\ninclude $${AM_HOME}/Makefile.app" > $@
	-make -f $@ ARCH=$(ARCH)

clean: 
	rm -rf Makefile.* build/

latest:
