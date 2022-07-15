include $(AM_HOME)/Makefile.check

.PHONY: all run clean $(ALL)

ALL = $(basename $(notdir $(shell find src/. -name "*.S")))

all: $(addprefix Makefile., $(ALL))
	echo $(ALL)

$(ALL): %: Makefile.%

Makefile.%: src/%.S
	@/bin/echo -e "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude $${AM_HOME}/Makefile.app" > $@
	-@make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS)
	-@rm -f Makefile.$*

clean:
	rm -rf Makefile.* build/

default $(MAKECMDGOALS): all ;
