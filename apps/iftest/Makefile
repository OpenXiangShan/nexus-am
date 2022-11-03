NAME_1 = iftest
CFLAGS +=-DCHOOSEO=$(CHOOSEO) -g 
CFLAGS +=-DCHOOSET=$(CHOOSET) -g
CFLAGS +=-DCHOOSER=$(CHOOSET) -g
CFLAGS +=-DCHOOSEF=$(CHOOSEF) -g
CFLAGS +=-DCHOOSEI=$(CHOOSEI) -g
NAME = build-$(NAME_1)-$(CHOOSEO)-$(CHOOSET)-$(CHOOSER)-$(CHOOSEF)-$(CHOOSEI)
$(NAME).c:iftest.c
	cp iftest.c $(NAME).c
SRCS = $(NAME).c
include $(AM_HOME)/Makefile.app
clean-build:
	rm -f build-*