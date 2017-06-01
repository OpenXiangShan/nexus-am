include $(AM_HOME)/Makefile.check
.PHONY: all clean latest $(ALL)

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))
	@echo "" $(ALL)

$(ALL): %: Makefile.%

Makefile.%: tests/%.c latest
	@echo "Build $*"
	@echo "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude $${AM_HOME}/Makefile.app" > $@
	-make -f $@ ARCH=$(ARCH)
	-rm -f Makefile.$*

clean: 
	rm -rf Makefile.* build/

latest:
