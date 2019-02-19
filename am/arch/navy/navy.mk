AM_SRCS := navy/trm.c \
           navy/platform.cpp \
           navy/ioe.c \
           navy/dev/input.c \
           navy/dev/timer.c \
           navy/dev/video.c

NAVY_MAKEFILE = Makefile.navy
NAVY_MAKEFILE_CONTENT = "NAME = $(NAME)-am\nAPP = $(BINARY_ABS)\nOBJS = $(LINK_FILES)\nLIBS += libndl\ninclude $${NAVY_HOME}/Makefile.app"

image:
	@/bin/echo -e $(NAVY_MAKEFILE_CONTENT) > $(NAVY_MAKEFILE)
	$(MAKE) install -f $(NAVY_MAKEFILE) ISA=$(ISA)
	rm $(NAVY_MAKEFILE)

run:
	echo "Cannot run: should be loaded by an OS"
